#include <M5Stack.h>
#include "TCPReceiver.h"

TCPReceiver tcpreceiver;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  Serial.begin(9600);
  M5.Lcd.fillScreen(0);
  M5.Lcd.setTextColor(0xFFFF, 0);
  M5.Lcd.drawString("WiFiwaiting", 0, 0);
  tcpreceiver.setup();
}

void loop() {
  // put your main code here, to run repeatedly:
  tcpreceiver.loop();
  M5.update();
}