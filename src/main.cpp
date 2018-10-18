#include "config.h"

#include <esphomelib.h>
#include <OneButton.h>

#if ENABLE_IR
#include <ESP32_IR_Remote.h>
#endif

#include <binary_state.h>

using namespace esphomelib;

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
}

void turnAcOnOff()
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
}

void switchAcState()
{
    acState->invert_state();
    mqttAcState->publish_state(acState->get_state());
}

void switchAutoAcOnOff()
{
    acAutoState->invert_state();
    mqttAcAutoState->publish_state(acAutoState->get_state());
}

void setupHomeAssistant()
{
    output::BinaryOutput *output = new BinaryState([](bool state) {
        static bool initialSync = true;
        if (initialSync) {
            initialSync = false;
            return;
        }
        
        turnAcOnOff();

        ESP_LOGCONFIG(TAG, "AC state: %d", state);
    });
    acState = (BinaryState *)output;

    output::BinaryOutput *autoOutput = new BinaryState([](bool state) {
        ESP_LOGCONFIG(TAG, "AC auto on/off state: %d", state);
    });
    acAutoState = (BinaryState *)autoOutput;

    auto acSwitch = App.make_simple_switch("AC", output);
    auto acAutoSwitch = App.make_simple_switch("AC Auto", autoOutput);

    auto sensor = App.make_dht_sensor("Living Temperature", "Living Humidity", DHT22_PIN, 2000);
    sensor.dht->set_dht_model(sensor::DHT_MODEL_DHT22);

    mqttAcState = acSwitch.mqtt;
    mqttAcAutoState = acAutoSwitch.mqtt;

    button.attachClick(switchAcState);
    button.attachDoubleClick(switchAutoAcOnOff);
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