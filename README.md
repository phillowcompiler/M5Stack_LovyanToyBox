M5Stack Lovyan ToyBox
===

M5Stack用 らびやんさんの実験的な何かで遊んだやつ。

## 追加ディレクトリ
* play_tcpreceiver M5Stack用プログラム。「ScreenShotReceiver」を改造したもの。
* ssender　Linux用のプログラム。Xvfbで動作するフレームバッファをjpegに変換してM5Stackに送り込むやつ。

## 解説
Linuxとかで仮想ディスプレイ（Xvfb）を作ってから、その上でmpvとかを走らせた状態で、仮想ディスプレイのフレームバッファからポコポコとスクリーンショットを取ってjpegに変換してM5Stackにwifi-ソケット通信（TCPプロトコル）で送る仕組み。

mpvはキーボードでも操作できるので、M5Stackの３つのボタンを適当に割り当てて操作できるようになっている。

Xvfbやmpvの解説や使い方はググるとザクザクと出てくるので、それを調べよう。

基本的にはraspberry piのようなARM上でもLinuxが動かせれば動かせるはずだけど、自分は初代ラズパイしか持っていないので、とてつもなく動作が重くて遊ぶどころではなくてまいった。

あくまでも遊ぶために作ったので、エラー処理とか汎用性については「何それ？おいしいの？」状態であります（＾_＾；


## 変更点

M5Stack側はtreeviewメニューをすっとばして、tcpreceiver単体で動くようになっています

元は「JPG」コマンドを送ると、画像データを受け取ることができるのだけど、「GET」で画像データ要求、「P〜」で始まるコマンドでボタン押下通知になっている。

* PAT Aボタン押下
* PAF Aボタンリリース
* PBT Bボタン押下
* PBF Bボタンリリース 
* PCT Cボタン押下
* PCF Cボタンリリース 

※だから、元プログラムのキー操作（下記　Usage　参照）は無効になっています。


## 遊んでて気づいたこと

M5Stack側の「tcpreceiver」はTCPクライアントではなくて、TCPサーバーになってる。

PlatformIOだと「tjpgdClass.cpp」の#include <arduino.h>でエラーになる。<Arduino.h>と書こう(^^;

jpeg圧縮率が高いままだと画像データが64kBを超えて死ねる（笑）。sender側のサイズは２バイトデータ指定で来る。

ソースだと、Wifi.begin()になってるので、最初に動かすときはSSIDとパスを指定する何かの工夫が必要だ（一回だけ、SSID-Passを書くとか）



# 以下はらびやん氏によるfork元の.md
## Description
  
M5Stackで作った諸々の詰め合わせです。  
  
* ScreenShotReceiver PCの画面をM5Stack上に表示  
* ScrollDemo   ili9341のスクロール機能を使ったミニゲーム  
* MPU9250Demo  9軸センサの動作デモ（GRAY/FIRE用）  
* ADInputDemo  アナログポートAD35/36の動作確認  
* FIRE MicFFT  FIREのマイク入力をFFT表示  
* IP5306 Registry 電源制御チップのI2Cレジストリの表示と編集  
  
※ OTA RollbackはSD-Updaterを使用している場合に別パテーションのアプリを起動します。  
  
実際の動きを以下のツイートの動画で見る事が出来ます。  
[機能紹介](https://mobile.twitter.com/lovyan03/status/1104025644202057729)  
[ScreenShotSender/Receiver](https://mobile.twitter.com/lovyan03/status/1104169374888255488)  
  
## Usage
 `BtnA click` : フォーカスを親へ移動  Move focus to parent item.  
 `BtnA hold`  : フォーカスを前へ移動  Move focus to the previous item.  
 `BtnB click` : 選択  Select focus item.  
 `BtnC click/hold` : フォーカスを次へ移動  Move focus to the next item.  
  
FACES/JoyStick/PLUSEncoder/CardKB にも対応しています。  
  
## Requirement
動作には以下のライブラリが必要です。  

* https://github.com/kosme/arduinoFFT/  
* https://github.com/tobozo/M5Stack-SD-Updater/  
* https://github.com/lovyan03/M5Stack_TreeView/  
* https://github.com/lovyan03/M5Stack_OnScreenKeyboard/  


## Licence

[LGPL v2.1](https://github.com/lovyan03/M5Stack_LovyanToyBox/blob/master/LICENSE)  

## Author

[lovyan03](https://twitter.com/lovyan03)  
