/**
 * Based on https://github.com/Darryl-Scott/ESP32-RMT-Library-IR-code-RAW
 */

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

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

// high, low, high, low, ...
const int data_ac[344] = {9120, -4592, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -8085, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -8064, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, 0};
int codelen = 343;

int buttonState = -1;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    digitalWrite(LED_BUILTIN, HIGH);
    irrecv.sendIR((int *)data_ac, codelen);
    digitalWrite(LED_BUILTIN, LOW);

    mqttClient.publish("ac/status", "on/off");
}

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);

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

            mqttClient.publish("ac/status", WiFi.localIP().toString().c_str());
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

    buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        irrecv.sendIR((int *)data_ac, codelen);
        digitalWrite(LED_BUILTIN, LOW);

        mqttClient.publish("ac/status", "on/off");
    }

    dht22loop();

    // if (millis() - lastDhtRead >= 10000)
    // {
    //     // DHT22: Check if any reads failed and exit early (to try again).
    //     // sensors_event_t event;

    //     // dht.temperature().getEvent(&event);
    //     // float t = event.temperature;
    //     // dht.humidity().getEvent(&event);
    //     // float h = event.relative_humidity;

    //     float t = 0, h = 0;
    //     if (isnan(t) || isnan(h))
    //     {
    //         Serial.println("Failed to read from DHT sensor!");
    //         mqttClient.publish("ac/status", "dht22 error");
    //     }
    //     else
    //     {
    //         String json = "{ \"temp\": ";
    //         json.concat(t);
    //         json.concat(", \"hum\": ");
    //         json.concat(h);
    //         json.concat("}");

    //         mqttClient.publish("sensors/dht22", json.c_str());
    //     }

    //     lastDhtRead = millis();
    // }
}
