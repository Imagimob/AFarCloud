/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// Interfaces for the optional BME280 sensor (not used for final demonstrator)

#ifndef __afarcloud_BME280
#define __afarcloud_BME280

#define I2C_SDA 21                            // SDA1
#define I2C_SCL 22                            // SCL1
#define BME280_ADDRESS 0x76                   // you can use I2C scanner demo to find your BME280 I2C address
#define DEFAULT_SEA_LEVEL_PRESSURE_HPA (1013) //hPA.


// This is NOT the standard library with the same name, but a modified version that with async methods
// Remove the original adafruit library and clone https://github.com/Imagimob/BME280-I2C-ESP32 into ~/Documents/Arduino/library
//
#include <Adafruit_BME280.h>

#include "config.h"

#ifdef DEBUG
#define DEBUGBME280
#endif

void initBME280();
void update_sea_level_pressure(float pressure);

void takeBME280_measurement_async();
bool isReadyBME280_measurement_async();
void waitBME280_measurement_ready();
void getBME280Values_async(float *temp, float *pressure, float *alt_barometric, float *hum);

void getBME280Values(float *temp, float *pressure, float *alt_barometric, float *hum);

#endif
