/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// Uncomment for debugging over serial link, comment for final firmware on unthetered device.
#define DEBUG

// Uncomment to disable LoRA functionalities so that one can dev without sending packets.
// #define OFFLINE_DEV

// Uncomment to enable additional BME280 sensor (Partial support only - data won't be sent)
// #define BME280

#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>

#include "config.h"
#include "definitions.h"
#include "LoRa.h"

#include "tbeam.h"
//include "gps.h"
#include "ring_buffer.h"
#include "payload_encoding.h"
#include "on_board_analytics.h"
#include "LoRa.h"
#include "accel.h"

#include <lmic.h>
#include <hal/hal.h>
//#include <WiFi.h>
#include <Wire.h>
//#include <TinyGPS++.h>
//#include <HardwareSerial.h>


#ifdef BME280
#include "BME280.h"
#endif


void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    setup_TTGO(false); //On new boards, true sets the board with gps turned on, false with gps off. On old boards, no effect.
#ifdef BME280
    initBME280();
#endif

    initAccel();
    //gps_initialize();

#ifndef OFFLINE_DEV
    setup_LoRa();
#endif

#ifdef DEBUG
    Serial.println("Fully initialized.");
#endif
}

bool got_fix = false;
bool fix_ok = false;
Fix new_fix;
Fix *lastFix = NULL;

inline long gps_lmic_loop_accel()
{
#ifndef OFFLINE_DEV
    if (LORA_sendjob_pending)
        os_runloop_once();
#endif
    //gps_pull_loop();
    return accel_loop();
}

void loop()
{
    //We'll be around here when we wake up from sleep

    int64_t esptime_last_sent_completed = esp_timer_get_time();
#define ESPTIME_SINCE_WAKEUP (((int64_t)esp_timer_get_time()) - esptime_wake_up)

    while (true)
    {
        //Small loop --- Query and sleep till it's time to send.

        const long int timeToNextAccelUpdate = gps_lmic_loop_accel();
        if (LORA_sendjob_pending)
            continue;

        if ((esp_timer_get_time() - esptime_last_sent_completed) > (TIME_BETWEEN_STATE_SEND * MICROSECOND_PER_SECONDS))
            break;

        if (timeToNextAccelUpdate > LIGHT_SLEEP_DELAY_MARGIN)
        {
            esp_sleep_enable_timer_wakeup(timeToNextAccelUpdate - LIGHT_SLEEP_DELAY_MARGIN);
            int ret = esp_light_sleep_start();
        }
    }

#ifdef BME280
    //Start an asynchronous measurement on the BME280
    takeBME280_measurement_async();
#endif

    //GPS&LMIC again
    gps_lmic_loop_accel();

    //Get battery voltage
    new_fix.battery_voltage = getBatteryVoltage();
    //Serial.print("Battery:");
    //Serial.println(new_fix.battery_voltage);

    //GPS&LMIC again
    gps_lmic_loop_accel();

#ifdef BME280
    delayMicroseconds(1); //The BME needs a tiny delay for the measurement to become available. 1us is enough. But without it, it never becomes available...
    while (!isReadyBME280_measurement_async())
    {
        gps_lmic_loop_accel();
        delayMicroseconds(1);
        //Serial.println("Waiting for BME...?");
    }

    getBME280Values_async(&(new_fix.temperature),
                          &(new_fix.pressure),
                          &(new_fix.alt_barometric),
                          &(new_fix.humidity));
#endif

    //GPS&LMIC again
    gps_lmic_loop_accel();

    //Construct payload
    build_payload_and_send(lastFix);
}
