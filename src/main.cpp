/**
 * Based on https://github.com/Darryl-Scott/ESP32-RMT-Library-IR-code-RAW
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <OneButton.h>

#include "ESP32_IR_Remote.h"
#include "dht.h"

const char *ssid = "***REMOVED***";
const char *password = "***REMOVED***";
const char *mqttServer = "***REMOVED***";

// MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

const int SEND_PIN = 26; // pin on the ESP32
const int BUTTON_PIN = 12;

ESP32_IRrecv irrecv;

OneButton button(BUTTON_PIN, true);

// high, low, high, low, ...
const int data_ac[344] = {9120, -4592, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -8085, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -8064, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, 0};
int codelen = 343;

int buttonState = -1;

int acState = 0;
int autoOnOff = 0;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String strTopic(topic);
    if (strTopic.equals("ac/control"))
    {
        digitalWrite(LED_BUILTIN, HIGH);
        irrecv.sendIR((int *)data_ac, codelen);
        digitalWrite(LED_BUILTIN, LOW);

        acState = acState ^ 1;

        mqttClient.publish("ac/status", acState == 1 ? "on" : "off", true);
    }
    else if (strTopic.equals("ac/status"))
    {
        String strPayload((char *)payload);
        if (strPayload.equals("on"))
        {
            acState = 1;
        }
        else if (strPayload.equals("off"))
        {
            acState = 1;
        }
    } else if (strTopic.equals("ac/auto")) {
        String strPayload((char *)payload);
        if (strPayload.equals("on")) {
            autoOnOff = 1;
        } else if (strPayload.equals("off")) {
            autoOnOff = 0;
        }
    }
}

void turnAcOnOff()
{
    digitalWrite(LED_BUILTIN, HIGH);
    irrecv.sendIR((int *)data_ac, codelen);
    digitalWrite(LED_BUILTIN, LOW);

    acState = acState ^ 1;
    mqttClient.publish("ac/status", acState == 1 ? "on" : "off", true);
}

void setAutoOnOff()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    autoOnOff = autoOnOff ^ 1;
    mqttClient.publish("ac/auto", autoOnOff == 1 ? "on" : "off", true);
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);

    button.attachClick(turnAcOnOff);
    button.attachDoubleClick(setAutoOnOff);

    // set 80 msec. debouncing time. Default is 50 msec.
    button.setDebounceTicks(50);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    mqttClient.setServer(mqttServer, 1883);
    mqttClient.setCallback(mqttCallback);

    irrecv.ESP32_IRsendPIN(SEND_PIN, 0);
    irrecv.initSend();
    delay(1000);

    Serial.println(codelen);

    dht22setup(&mqttClient);
}

void reconnect()
{
    // Loop until we\"re reconnected
    while (!mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (mqttClient.connect("esp32irblaster"))
        {
            Serial.println("connected");

            // Await commands on this topic
            mqttClient.subscribe("ac/control");
            // Sync state
            mqttClient.subscribe("ac/status");

            mqttClient.publish("ac/esp32-wifi", WiFi.localIP().toString().c_str());
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

int lastDhtRead = 0;

void loop()
{
    // Connect MQTT
    if (!mqttClient.connected())
    {
        reconnect();
    }
    mqttClient.loop();

    button.tick();

    dht22loop();
}
