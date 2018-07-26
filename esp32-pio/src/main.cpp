/**
 * Based on https://github.com/Darryl-Scott/ESP32-RMT-Library-IR-code-RAW
 */

#include "Arduino.h"
#include "ESP32_IR_Remote.h"

const int SEND_PIN = 26; // pin on the ESP32
const int BUTTON_PIN = 12;

ESP32_IRrecv irrecv;

#define SendIRxTimes 1

unsigned int IRdata[1000]; //holding IR code in ms

// high, low, high, low, ...
const unsigned int data_ac[] = {2581, 917, 405, 875, 427, 448, 405, 469, 384, 917, 832, 469, 405, 469, 405, 448, 384, 469, 384, 469, 384, 469, 405, 469, 405, 469, 384, 469, 832, 469, 384, 896, 853, 469, 384, 896, 853, 84309, 2581, 917, 405, 896, 405, 469, 363, 512, 341, 917, 832, 512, 341, 512, 341, 512, 341, 512, 341, 512, 341, 512, 341, 512, 341, 512, 341, 491, 811, 512, 341, 917, 853, 491, 341, 917, 853};
int codelen = sizeof(data_ac);

int buttonState = 0;

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    irrecv.ESP32_IRsendPIN(SEND_PIN, 0);
    irrecv.initSend();
    delay(1000);
    
    Serial.println(codelen);
}

void loop()
{

    buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW)
    {
        irrecv.sendIR((unsigned int *)data_ac, codelen);
    }
}
