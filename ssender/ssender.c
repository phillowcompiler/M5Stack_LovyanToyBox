//
//  ssender.cpp
//
//  426x240の仮想ディスプレイのフレームバッファをポコポコとjpegで送るやつ
//　Xvfb :1 -screen 0 426x240x16 -fbdir /tmp ←このように書くと/tmp/Xvfb_screen0というフレームバッファができる
//　
//  compile: gcc -std=c11 ssender.c -ljpeg -lX11 -lXtst -o ssender
//  run    :$./ssender xxx.yyy.zzz(Soc IP)
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <jpeglib.h>
#include <sys/stat.h>

#include <X11/extensions/XTest.h>
#include <X11/keysym.h>


//ファイルサイズ取得
long getFileSize(char *file)
{
    struct stat statBuf;
 
    if ( stat(file, &statBuf) == 0 )
        return statBuf.st_size;
 
    return -1L;
}

//文字列比較
int charcmp( char *in1, char *in2, int len ){
    while( len ){
        if( *in1++ != *in2++ )break;
        len--; 
    }
    return len;
}

//Fake Input
int fakeinput(KeySym ks, char sts){
    Display *dis;
    dis = XOpenDisplay(":1");
    KeyCode kc =  XKeysymToKeycode( dis, ks );
    //XTestFakeKeyEvent(dis,  kc, False, 0);
    //XFlush(dis);
    if( sts == 'T' ){
        XTestFakeKeyEvent(dis, kc, True, 0);
        XFlush(dis);
    }
    else if( sts == 'F' ){
        XTestFakeKeyEvent(dis, kc, False, 0);
        XFlush(dis);
    }
    XCloseDisplay(dis);
    return 0;
}


//フェイクキー押下処理
int putkey(char key, char sts){
    KeySym ks;
    char tmp[3];

    switch(key){
    case 'A':
        ks =  XK_Left;
        break;
    case 'B':
        ks = XK_space;
        //ks = XK_Up;
        break;
    case 'C':
        ks =  XK_Right;
        break;
    default:
        return -1;
    }
    tmp[0] = key;
    tmp[1] = sts;
    tmp[2] = 0;
    printf("KEY:%s\n",tmp);
    fakeinput(ks, sts);
    return 0;
}

//コマンド受信待ち
int waitcmd(int sock){
    char _buffer[16];
    int retry = 0;
    int count = 0;
    
    while( retry < 10000 ){
        //データ受信
        if( read(sock, _buffer + count, 1) > 0 ){
            if( _buffer[count-1] != '\r' ){
                if( ++count > sizeof(_buffer)-2 ){return 0;}
            }else{
                //改行コードの受信
                if( _buffer[count++] == '\n'){
                    if( count >= 5 ){
                        if( !charcmp(_buffer,"GET",3) ){
                            //printf("GET\n");                            
                            return 0;
                        }
                        if( _buffer[0]=='P' ){
                            putkey( _buffer[1], _buffer[2] );
                            return 0;
                        }
                        printf("get illegal cmd!\n");
                        return 0;
                    }           
                }
                count = 0;
            }          
        }
        retry++;
        usleep(100);
    }
    printf("cmd time out\n");
    return 0;
}



//source geometry
#define INPUTW 426
#define INPUTH 240

//output geometry
#define OUTW 320
#define OUTH 240


int getImage () {
    /* JPEGオブジェクト, エラーハンドラの確保 */
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    int width = 320;
    int height = 240;
    
    /* 入力ファイル */
    char *ifname = "/tmp/Xvfb_screen0";
    FILE *fpi = fopen(ifname,"rb");
    if (fpi == NULL) {
        fprintf(stderr, "cannot open file\n");
        exit(EXIT_FAILURE);
    }
    /* サイズ取得 */
    long size = getFileSize(ifname);
    //printf("input file size=%ld\n",size);
    
    char *in = malloc(size);
    fseek(fpi, 0x3A0, SEEK_SET);
    fread(in,2,INPUTW*INPUTH,fpi);
    fclose(fpi);

    /* エラーハンドラにデフォルト値を設定 */
    cinfo.err = jpeg_std_error(&jerr);

    /* JPEGオブジェクトの初期化 */
    jpeg_create_compress(&cinfo);
    
    /* 出力ファイルの設定 */
    char *filename = "output.jpg";
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "cannot open %s\n", filename);
        exit(EXIT_FAILURE);
    }
    jpeg_stdio_dest(&cinfo, fp);

    /* 画像のパラメータの設定 */
    cinfo.image_width = OUTW;
    cinfo.image_height = OUTH;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 75, TRUE);

    /* 圧縮開始 */
    jpeg_start_compress(&cinfo, TRUE);

     /* RGB値の設定 */
    JSAMPARRAY img = (JSAMPARRAY) malloc(sizeof(JSAMPROW) * height);

    unsigned short *p = (unsigned short*)in;    

    for (int i = 0; i < height; i++) {
        img[i] = (JSAMPROW) malloc(sizeof(JSAMPLE) * 3 * width);
        p += (INPUTW-width)/2;
        for (int j = 0; j < width; j++) {
            unsigned short c = *p++;      
            img[i][j*3 + 0] = (c&0xf800) >> 8;  /* R */
            img[i][j*3 + 1] = (c&0x7e0) >> 3;  /* G */
            img[i][j*3 + 2] = (c&0x1f)<<3;     /* B */   

        }
        p += (INPUTW-width)/2;
    }
    free(in);
    /* 書き込む */
    jpeg_write_scanlines(&cinfo, img, height);

    /* 圧縮終了 */
    jpeg_finish_compress(&cinfo);

    /* JPEGオブジェクトの破棄 */
    jpeg_destroy_compress(&cinfo);

    for (int i = 0; i < height; i++) {
        free(img[i]);
    }
    free(img);
    fclose(fp);
}

long putImage(char **out){
     /* 入力ファイル */
    char *ifname = "output.jpg";
    FILE *fpi = fopen(ifname,"rb");
    if (fpi == NULL) {
        fprintf(stderr, "cannot open file\n");
        exit(EXIT_FAILURE);
    }
    /* サイズ取得 */
    long len = getFileSize(ifname);
    *out = malloc(len+0x10);
    memcpy(*out,"JPG",3);
    *(*out + 3) = (char)(len & 0xFF);
    *(*out + 4) = (char)((len >> 8) & 0xFF);

    fread(*out + 5, 1, len, fpi);
    fclose(fpi);
    return len+5;    
}


//
//  Main.
//
int main(int argc, char **argv)
{
    int sock0;
    struct sockaddr_in client_addr;
    char *snd;


    //ソケット作成
    sock0 = socket(AF_INET, SOCK_STREAM, 0);

    //ソケット作成エラーチェック
    if(sock0 < 0) {
        perror("socket");
        return -1;
    }


    bzero((char *)&client_addr, sizeof(client_addr));
    client_addr.sin_family = PF_INET;
    
    if(!argc){
    client_addr.sin_addr.s_addr =  inet_addr("192.168.0.111");
    }else{
    client_addr.sin_addr.s_addr =  inet_addr(argv[1]);
    printf("SERVER=%s\n",argv[1]);
    }
    client_addr.sin_port = htons(63333);

    /* ソケットをサーバに接続 */
    if (connect(sock0, (struct sockaddr *)&client_addr, sizeof(client_addr)) > 0) {
        perror("client: connect");
        close(sock0);
        exit(1);
      }
    printf("connected\n");

    //接続要求待ちループ
    while (1) {
        //コマンド受信待ち
        if( waitcmd(sock0) >= 0)
        {
        //スクリーンショットの取得と送信        
        getImage();
        long len = putImage(&snd);
        //printf("sndsize=%d\n",len);
        
        //データ送信
        send(sock0, (char *)snd, len, 0);
        free(snd);
        }

    }

    //ソケットクローズ
    close(sock0);
    printf( "Closed socket.\n" );

    return 0;
}

