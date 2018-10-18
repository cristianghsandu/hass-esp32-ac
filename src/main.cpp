#include "config.h"

#include <esphomelib.h>
#include <OneButton.h>

#if ENABLE_IR
#include <ESP32_IR_Remote.h>
#endif

#include <binary_state.h>

using namespace esphomelib;

// Config
const int SEND_PIN = 26;
const int BUTTON_PIN = 12;
const int DHT22_PIN = 14;

const char *ssid = "";
const char *password = "";
const char *mqttServer = "***REMOVED***";

// high, low, high, low, ...
const int data_ac[344] = {9120, -4592, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -8085, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -8064, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, 0};
int codelen = 343;

#if ENABLE_IR
ESP32_IRrecv irrecv;
#endif

OneButton button(BUTTON_PIN, true);

switch_::MQTTSwitchComponent *mqttAcState;
BinaryState *acState;

switch_::MQTTSwitchComponent *mqttAcAutoState;
BinaryState *acAutoState;

void setAutoOnOff()
{
    if (!acAutoState || !mqttAcAutoState)
    {
        return;
    }

    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);

    acAutoState->invert_state();
    mqttAcAutoState->publish_state(acAutoState->get_state());
}


void turnAcOnOff(bool publish = true)
{
    if (!acState || !mqttAcState)
    {
        return;
    }

    digitalWrite(LED_BUILTIN, HIGH);
#if ENABLE_IR
    irrecv.sendIR((int *)data_ac, codelen);
#endif
    digitalWrite(LED_BUILTIN, LOW);

    if (publish)
    {
        acState->invert_state();
        mqttAcState->publish_state(acState->get_state());
    }
}

void onButtonClick()
{
    turnAcOnOff();
}

void setupHomeAssistant()
{
    output::BinaryOutput *output = new BinaryState([](bool state) {
        Serial.print("AC: ");
        Serial.println(state);
        // turnAcOnOff(false); // Publishing the state back to MQTT will create an infinite loop
    });
    acState = (BinaryState *)output;

    output::BinaryOutput *autoOutput = new BinaryState([](bool state) {
        Serial.print("AC Auto: ");
        Serial.println(state);
    });
    acAutoState = (BinaryState *)autoOutput;

    auto acSwitch = App.make_simple_switch("AC", output);
    auto acAutoSwitch = App.make_simple_switch("AC Auto", autoOutput);

    auto sensor = App.make_dht_sensor("Living Temperature", "Living Humidity", DHT22_PIN, 2000);
    sensor.dht->set_dht_model(sensor::DHT_MODEL_DHT22);

    mqttAcState = acSwitch.mqtt;
    mqttAcAutoState = acAutoSwitch.mqtt;

    button.attachClick(onButtonClick);
    button.attachDoubleClick(setAutoOnOff);
}

void setupPins()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // Debouncing time
    button.setDebounceTicks(50);

#if ENABLE_IR
    irrecv.ESP32_IRsendPIN(SEND_PIN, 0);
    irrecv.initSend();
#endif
}

void setup()
{
    App.set_name("AC-Living");
    App.init_log();

    App.init_wifi("***REMOVED***", "***REMOVED***");
    App.init_ota()->start_safe_mode();
    App.init_mqtt("***REMOVED***", "", "");
    App.init_web_server();

    setupHomeAssistant();

    setupPins();

    App.setup();
}

void loop()
{
    button.tick();

    App.loop();
}