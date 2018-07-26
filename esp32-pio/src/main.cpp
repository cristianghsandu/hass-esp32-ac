/**
 * Based on https://github.com/Darryl-Scott/ESP32-RMT-Library-IR-code-RAW
 */

#include "Arduino.h"
#include "ESP32_IR_Remote.h"

const int SEND_PIN = 26; // pin on the ESP32

ESP32_IRrecv irrecv;

#define SendIRxTimes 1

unsigned int IRdata[1000]; //holding IR code in ms

void setup() {
  Serial.begin(115200);
}

void loop() {
    
}
