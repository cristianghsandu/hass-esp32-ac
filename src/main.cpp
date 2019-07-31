#include "config.h"

#include <esphome.h>
#include <OneButton.h>

#if ENABLE_IR
#include <ESP32_IR_Remote.h>
#endif

#include <binary_state.h>

using namespace esphome;

#if ENABLE_IR
ESP32_IRrecv irrecv;
#endif

OneButton button(BUTTON_PIN, true);

switch_::MQTTSwitchComponent *mqttAcState;
BinaryState *acState;

void turnAcOnOff()
{
    if (!acState || !mqttAcState)
    {
        return;
    }

    digitalWrite(LED_BUILTIN, HIGH);
#if ENABLE_IR
    irrecv.sendIR((int *)data_ac, data_ac_len);
#endif
    digitalWrite(LED_BUILTIN, LOW);
}

void switchAcState()
{
    acState->invert_state();
    mqttAcState->publish_state(acState->get_state());
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

    auto acSwitch = App.make_output_switch("AC", output);

    auto sensor = App.make_dht_sensor("Living Temperature", "Living Humidity", DHT22_PIN, 2000);
    sensor->set_dht_model(sensor::DHT_MODEL_DHT22);

    mqttAcState = new switch_::MQTTSwitchComponent(acSwitch);

    button.attachClick(switchAcState);
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

    App.init_wifi(ssid, password);
    App.init_ota()->start_safe_mode();
    App.init_mqtt(mqttServer, "", "");
    App.init_api_server();

    setupHomeAssistant();

    setupPins();

    App.setup();
}

void loop()
{
    button.tick();

    App.loop();
}