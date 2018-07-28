#include <PubSubClient.h>

#include "DHTesp.h"
#include "Ticker.h"

#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY !)
#error Select ESP32 board.
#endif

static DHTesp dht;

static PubSubClient *mqttClient;

static void tempTask(void *pvParameters);
static bool getTemperature();
static void triggerGetTemp();

/** Task handle for the light value read task */
static TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
static Ticker tempTicker;
/** Comfort profile */
static ComfortState cf;
/** Flag if task should run */
static bool tasksEnabled = false;
/** Pin number for DHT22 data pin */
static int dhtPin = 14;

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
static bool initTemp()
{
    // Initialize temperature sensor
    dht.setup(dhtPin, DHTesp::DHT22);
    Serial.println("DHT initiated");

    // Start task to get temperature
    xTaskCreatePinnedToCore(
        tempTask,        /* Function to implement the task */
        "tempTask ",     /* Name of the task */
        4000,            /* Stack size in words */
        NULL,            /* Task input parameter */
        5,               /* Priority of the task */
        &tempTaskHandle, /* Task handle. */
        1);              /* Core where the task should run */

    if (tempTaskHandle == NULL)
    {
        Serial.println("Failed to start task for temperature update");
        return false;
    }
    else
    {
        // Start update of environment data every 20 seconds
        tempTicker.attach(20, triggerGetTemp);
    }
    return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
static void triggerGetTemp()
{
    if (tempTaskHandle != NULL)
    {
        xTaskResumeFromISR(tempTaskHandle);
    }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
static void tempTask(void *pvParameters)
{
    Serial.println("tempTask loop started");
    while (1) // tempTask loop
    {
        if (tasksEnabled)
        {
            // Get temperature values
            getTemperature();
        }
        // Got sleep again
        vTaskSuspend(NULL);
    }
}

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
static bool getTemperature()
{
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    TempAndHumidity newValues = dht.getTempAndHumidity();
    // Check if any reads failed and exit early (to try again).
    if (dht.getStatus() != 0)
    {
        Serial.println("DHT22 error status: " + String(dht.getStatusString()));

        if (mqttClient)
        {
            mqttClient->publish("ac/status", "dht22 error");
        }

        return false;
    }

    float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
    float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
    float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

    String comfortStatus;
    switch (cf)
    {
    case Comfort_OK:
        comfortStatus = "Comfort_OK";
        break;
    case Comfort_TooHot:
        comfortStatus = "Comfort_TooHot";
        break;
    case Comfort_TooCold:
        comfortStatus = "Comfort_TooCold";
        break;
    case Comfort_TooDry:
        comfortStatus = "Comfort_TooDry";
        break;
    case Comfort_TooHumid:
        comfortStatus = "Comfort_TooHumid";
        break;
    case Comfort_HotAndHumid:
        comfortStatus = "Comfort_HotAndHumid";
        break;
    case Comfort_HotAndDry:
        comfortStatus = "Comfort_HotAndDry";
        break;
    case Comfort_ColdAndHumid:
        comfortStatus = "Comfort_ColdAndHumid";
        break;
    case Comfort_ColdAndDry:
        comfortStatus = "Comfort_ColdAndDry";
        break;
    default:
        comfortStatus = "Unknown:";
        break;
    };

    Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint) + " " + comfortStatus);

    if (mqttClient)
    {
        String json = "{ \"temp\": ";
        json.concat(newValues.temperature);
        json.concat(", \"hum\": ");
        json.concat(newValues.humidity);
        json.concat("}");

        mqttClient->publish("sensors/dht22", json.c_str());
    }

    return true;
}

void dht22setup(PubSubClient *client)
{
    initTemp();
    // Signal end of setup() to tasks
    tasksEnabled = true;

    mqttClient = client;
}

void dht22loop()
{
    if (!tasksEnabled)
    {
        // Wait 2 seconds to let system settle down
        delay(2000);
        // Enable task that will read values from the DHT sensor
        tasksEnabled = true;
        if (tempTaskHandle != NULL)
        {
            vTaskResume(tempTaskHandle);
        }
    }
}
