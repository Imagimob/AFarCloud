/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

/***************************************************************
 *
 *       Interfaces for GPS functionalities
 *
 ***************************************************************/

#ifndef __afarcloud_GPS
#define __afarcloud_GPS

#include <Arduino.h>
#include <TinyGPS++.h>
#include "tbeam.h"
#include "definitions.h"
#include "config.h"

void gps_initialize(); // Initializes GPS
void gps_pull_loop();  // GPS routine that reads from serial buffer.

bool checkGpsFix();        // Check if a fix has been received
bool gps_getFix(Fix *fix); // Read the fix ---- MUST check checkGpsFix()==True

void gps_Update_and_GetFix(Fix *fix); // Blocking version of the above. Will block till fix is ready and read it.

#ifdef DEBUG
void printFix(Fix *fix);
void printFix_full(Fix *fix);
#endif

#endif //__afarcloud_GPS
