#include <esphomelib.h>

// Config
const int SEND_PIN = 26;    // pin on the ESP32
const int BUTTON_PIN = 12;


const char *ssid = "";
const char *password = "";
const char *mqttServer = "***REMOVED***";

// high, low, high, low, ...
const int data_ac[344] = {9120, -4592, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -8085, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -1745, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -1745, 555, -1745, 555, -8064, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -1745, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, -555, 555, 0};
int codelen = 343;

using namespace esphomelib;

void setup() {
    App.set_name("AC-Living");
    App.init_log();

    App.init_wifi("***REMOVED***", "***REMOVED***");
    App.init_ota()->start_safe_mode();
    App.init_mqtt("***REMOVED***", "", "");
    App.init_web_server();

    // auto *red = App.make_ledc_output(32); // on pin 32, only available with ESP32
    // auto *green = App.make_ledc_output(33);
    // auto *blue = App.make_ledc_output(34);
    // App.make_rgb_light("Livingroom Light", red, green, blue);
    
    // App.make_dht_sensor("Livingroom Temperature", "Livingroom Humidity", 12);

    App.setup();
}

void loop() {
    App.loop();
}
