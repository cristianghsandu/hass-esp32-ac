#include "config.h"

#include <esphomelib.h>
#if ENABLE_IR
#include <ESP32_IR_Remote.h>
#endif

#include "binary_state.h"

using namespace esphomelib;

// Config
const int SEND_PIN = 26;
const int BUTTON_PIN = 12;
const int DHT22_PIN = 14;

const char *ssid = "";
const char *password = "";
const char *mqttServer = "***REMOVED***";

const BinaryState* acAutoState = NULL;

// high, low, high, low, ...
const int data_ac[344] = {9120, -4592, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -8085, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -8064, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, 0};
int codelen = 343;

#if ENABLE_IR
ESP32_IRrecv irrecv;
#endif


void setupSensors()
{
    auto sensor = App.make_dht_sensor("Living Temperature", "Living Humidity", DHT22_PIN, 2000);
    sensor.dht->set_dht_model(sensor::DHT_MODEL_DHT22);

    auto pushButton = App.make_gpio_binary_sensor("AC Push Button", GPIOInputPin(BUTTON_PIN, INPUT_PULLUP));
    auto clickTrigger = pushButton.gpio->make_click_trigger(0, 2000);
    clickTrigger->add_on_trigger_callback([](bool state) {
        Serial.println("Single click, man");
    });
}

void makeStateButtons()
{
    output::BinaryOutput* output = new BinaryState([](bool state) {
        Serial.print("Auto: ");
        Serial.println(state);
    });
    App.make_simple_switch("AC Auto", output);
    
    // Save it
    acAutoState = (BinaryState*)output;
}

void turnAcOnOff()
{
    digitalWrite(LED_BUILTIN, HIGH);
#if ENABLE_IR
    irrecv.sendIR((int *)data_ac, codelen);
#endif
    digitalWrite(LED_BUILTIN, LOW);
}

void setup()
{
    App.set_name("AC-Living");
    App.init_log();

    App.init_wifi("***REMOVED***", "***REMOVED***");
    App.init_ota()->start_safe_mode();
    App.init_mqtt("***REMOVED***", "", "");
    App.init_web_server();

    setupSensors();
    makeStateButtons();

    App.setup();
}

void loop()
{
    App.loop();
}