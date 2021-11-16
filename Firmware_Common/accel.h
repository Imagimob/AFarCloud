/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// Interfaces for accessing accelerometer data, pushing data to the neural network and querying the result
#ifndef __accel_h
#define __accel_h

#include "config.h"
#include "definitions.h"
#include <BMI160Gen.h> //Clone https://github.com/Imagimob/BMI160-Arduino into ~/Documents/Arduino/libraries/

// Definition file for the neural network model from Imagimob dev platform
extern "C"
{
#include "model.h"
};

#ifdef DEBUG
#define DEBUGACCEL
#endif


const int i2c_addr = 0x69; // BMI160 hardware address

const int gyro_scale = 500; // supported values: 125, 250, 500, 1000, 2000 (degrees/second)
const int accel_scale = 8;  // supported values: 2, 4, 8, 16 (G)

// These parameters needs to be identical to those used for data collection
const int gyro_rate = 50;    // supported values: 25, 50, 100, 200, 400, 800, 1600, 3200 (Hz)
const float accel_rate = 50; // supported values: 12.5, 25, 50, 100, 200, 400, 800, 1600 (Hz)

void initAccel(); // Call this at the begining of the program for initializing sensor and NN model

const long int accel_loop(); // To be called repeatadly during runtime, at least accel_rate x / s

// Primitives for reading the states fraction
long int getAccumulatorValue(const int pattern);
long int getAccumulatorTime();
void resetAccumulator();

#endif //__accel_h