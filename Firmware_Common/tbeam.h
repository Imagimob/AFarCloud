/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

/**************************************************************
 *
 *         T-beam TT-GO board-specific parameter
 *
 *
 *
 ***************************************************************/

#ifndef __afarcloud_tbeam
#define __afarcloud_tbeam

#include <Arduino.h>
#include "config.h"
#include <stdint.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <WiFi.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Pin mapping
extern const lmic_pinmap lmic_pins;

float getBatteryVoltage();
int8_t getGPSRXpin();
int8_t getGPSTXpin();

//Sets up the board. False turns off power to the GPS on new board (no effect on old boards)
void setup_TTGO(bool);

//Control the blue led (not sure this works on new boards)
inline void blue_led_on()
{
    digitalWrite(BUILTIN_LED, HIGH);
}

inline void blue_led_off()
{
    digitalWrite(BUILTIN_LED, LOW);
}

#ifdef DEBUG
void printBatteryVoltage();
#endif //DEBUG

#endif __afarcloud_tbeam
