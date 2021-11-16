/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// Uncomment for debugging over serial link, comment for final firmware on unthetered device.
#define DEBUG

// Uncomment to disable LoRA functionalities so that one can dev without sending packets.
// #define OFFLINE_DEV

// Uncomment to enable additional BME280 sensor (Partial support only - data won't be sent)
//#define BME280

#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>

#include "config.h"
#include "definitions.h"
#include "LoRa.h"

#include "tbeam.h"
#include "gps.h"
#include "ring_buffer.h"
#include "payload_encoding.h"
#include "on_board_analytics.h"
#include "LoRa.h"
#include "accel.h"

#include <lmic.h>
#include <hal/hal.h>
//#include <WiFi.h>
#include <Wire.h>
#include <TinyGPS++.h>
//#include <HardwareSerial.h>

#ifdef BME280
#include "BME280.h"
#endif

void setup()
{
#ifdef DEBUG
    Serial.begin(115200);
#endif

    setup_TTGO(true); //On new boards, true sets the board with gps turned on, false with gps off. On old boards, no effect.
#ifdef BME280
    initBME280();
#endif

    initAccel();
    gps_initialize();

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

inline void gps_lmic_loop_accel()
{
#ifndef OFFLINE_DEV
    if (LORA_sendjob_pending)
        os_runloop_once();
#endif
    gps_pull_loop();
    accel_loop();
}

void loop()
{
    //We'll be around here when we wake up from sleep

    int64_t esptime_wake_up = esp_timer_get_time();
#define ESPTIME_SINCE_WAKEUP (((int64_t)esp_timer_get_time()) - esptime_wake_up)

    //Check the GPS and run the LMIC loop if needed
    gps_lmic_loop_accel();

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

    //We might have a fix from before sleep --- now, we decide if we push it to the ring buffer or not

    if ((got_fix) && (fix_ok))
    {

        //printFix_full(&new_fix);
        lastFix = &new_fix;
        if (gotoLast())
        {
            // Get the previous pushed fix and compare distance
            Fix prev_pushed_fix;
            readFix(prev_pushed_fix);

#ifdef DEBUG
            Serial.print("Got a good fix!");
            Serial.print("Distance from previous fix: ");
            Serial.println(getDistanceBetweenFixes(&prev_pushed_fix, &new_fix));
            Serial.print("Time since previous fix: ");
            Serial.println(getTimeBetweenFixes(&prev_pushed_fix, &new_fix));
#endif
            if ((getDistanceBetweenFixes(&prev_pushed_fix, &new_fix) > GPS_MIN_DISTANCE) ||
                (getTimeBetweenFixes(&prev_pushed_fix, &new_fix) > MAX_GPS_TIME_BETWEEN_FIXES))
            {
                forcePush(new_fix);
                lastFix = NULL;
            }
        }
        else
        {
            forcePush(new_fix); //Buffer is empty, so we push the fix.
            lastFix = NULL;
        }
    }
    else
    {
#ifdef DEBUG
        Serial.println("###Got invalid fix");
    }
#endif

    //GPS&LMIC again
    gps_lmic_loop_accel();

    //Run the line simplification algorithm on the buffer and construct a payload to be ready in case we decide to send
    //We anyway have ~250 ms at full CPU to wait for the GPS Serial link to re-establish :-/

    //TBA: line simplication

    /* NB for self: at some point, I though that the below isn't needed if no fix was pushed in this loop...
     * That is not true.
     *
     * build_payload and decide2send are themselves changing the buffer. It could very well be that we
     * have accumulated lots of point in the buffer and need to empty, thus sending multiple times without adding
     *
     */

    //Construct payload
    build_payload_and_send(lastFix);

    //GPS&LMIC again
    gps_lmic_loop_accel();

#ifdef BME280

    //Serial.println("Waiting for BME");
    //Wait for BME measurement to be ready
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

    /* Serial.print("Done all computing in ");
    Serial.print((int64_t) (esp_timer_get_time()-wake_up_time));
    Serial.println(" us");
    */

    //Because I do not run line simplification for now, set all fixes to corners
    new_fix.corner = true;

    //douglasPeuckerClassify();
    //#ifdef DEBUG
    //    Serial.print("Time to run all the code: ");
    //    Serial.print(ESPTIME_SINCE_WAKEUP);
    //    Serial.println(" us");
    //#endif //DEBUG

    //Get the fix
    do
    {
        got_fix = gps_getFix(&new_fix);
        gps_lmic_loop_accel();

        if (ESPTIME_SINCE_WAKEUP > 1250000) //If we didn't get a fix after 1s, we give up
            break;
    } while (!got_fix);

    fix_ok = checkGpsFix();

    int64_t esptime_fix = esp_timer_get_time();
#define ESPTIME_SINCE_FIX (((int64_t)esp_timer_get_time()) - esptime_fix)

    /* We might have got a fix (or we timed-out)
     * If we have a fix, we need to decided if we keep that one or not.
     * Since we have 250 ms of doing nothing at wake up, we sleep directly and decide at wake up
     */

    // #ifdef DEBUG
    //     Serial.print("Time to fix: ");
    //     Serial.print(ESPTIME_SINCE_WAKEUP);
    //     Serial.println(" us");
    // #endif //DEBUG

    while ((LORA_sendjob_pending) && (ESPTIME_SINCE_WAKEUP < (GPS_TIME_BETWEEN_FIXES * MICROSECOND_PER_SECONDS)))
        gps_lmic_loop_accel();

#define TIME_TO_NEXT_GPS_FIX ((GPS_TIME_BETWEEN_FIXES * MICROSECOND_PER_SECONDS) - ESPTIME_SINCE_FIX - 250000)

    while (TIME_TO_NEXT_GPS_FIX > 0)
    {
        if (LORA_sendjob_pending)
            os_runloop_once();

        const long int timeToNextAccelUpdate = accel_loop();

        if (timeToNextAccelUpdate > LIGHT_SLEEP_DELAY_MARGIN)
        {
            esp_sleep_enable_timer_wakeup(timeToNextAccelUpdate - LIGHT_SLEEP_DELAY_MARGIN);
            int ret = esp_light_sleep_start();
        }
    }
}

/*
 *
 *
    //Serial.println((int64_t) esp_timer_get_time()) ;

   float temp, pressure, barometric_altitude, humidity;

   //takeBME280_measurement_async();
   //We have 16-20 ms for doing something else while the sensor is measuring

   //float batt = getBatteryVoltage();  //This takes 9-12 us

   //delay(20);
   //esp_sleep_enable_timer_wakeup(20000); // If we light-sleep instead, it doesn't work! Still need to wait ~16 us at wakeup :-(



   //waitBME280_measurement_ready();

   //getBME280Values_async(&temp, &pressure, &barometric_altitude, &humidity);
   //delay(10);
   // printBatteryVoltage();
    Fix fix;
    int64_t before = esp_timer_get_time();
    gps_Update_and_GetFix(&fix);  //About 250ms to simply read the data from the serial port is as good as it gets :(
    int64_t after = esp_timer_get_time();
    Serial.print("Fix in ");
    Serial.print(after-before);
    Serial.println(" us");


    //Serial.println("light_sleep_enter");

    //esp_sleep_enable_timer_wakeup(1*MICROSECOND_PER_SECONDS-(end_of_job_time-wakeup_time));
    esp_sleep_enable_timer_wakeup(750000);
    int ret = esp_light_sleep_start();
    //delay(500);

   //Serial.printf("light_sleep: %d\n", ret);

 */
