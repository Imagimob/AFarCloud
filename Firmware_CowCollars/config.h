/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/


#ifndef __AFARCLOUD_CONFIG
#define __AFARCLOUD_CONFIG


/***************************************************************
 *
 *         LoRaWAN parameters
 *
 *
 *
 **************************************************************
 */


//Create your own secrets.h following the example secrets.example.h
#include "secrets.h"

/* Data rate
  Data  Rate          Configuration bits/s  Max payload
  DR0   SF12/125kHz   250                   59
  DR1   SF11/125kHz   440                   59
  DR2   SF10/125kHz   980                   59
  DR3   SF9/125kHz    1 760                 123
  DR4   SF8/125kHz    3 125                 230
  DR5   SF7/125kHz    5 470                 230
  DR6   SF7/250kHz    11 000                230
  DR7   FSK: 50kpbs   50 000                230
*/

//TTN should be SF9 --- for now, don't touch!
#define DOWNLINK_DATA_RATE DR_SF9
#define UPLINK_DATA_RATE DR_SF10

//Unsure what this does for now. Leave it at 14
#define UPLINK_POWER_LEVEL 14


// Controls the line simplification process on how far off a fix can be from the "line" in order to be discarded.
const float EPSILON = 15.0;

//Size of the ring buffer in number of GPS point to store (at a rate of 1 fix/sec)
#define BUFFER_SIZE 720 // 500 is probably too much




// NB: for cow collars, no gps --- all these parameters are ignored
// ########### Define minimum requires for a fix to be accepted ##########
#define MIN_SATELLITES 5
#define MAX_HDOP 200 //HDOP 200 = 2.0
#define MAX_GPS_AGE 1000


// ########### Define overall behavior of the tracker ##########

#define GPS_TIME_BETWEEN_FIXES  5 //in seconds - interval for (trying) to pull a GPS fix.

#define GPS_MIN_DISTANCE 20 //in meters --- Ignore points closer than this to last recorded fix.
#define MAX_GPS_TIME_BETWEEN_FIXES 1800 //in seconds - interval for registering a fix even if we haven't moved.


//Don't while in EU
#define CFG_eu868 1

//Number of states for the NN
#define NUMBER_STATES 4


// Safety margin for the timing for light sleep.
// Seems to be 250 us to get to light sleep and 1 ms +/- 5% to wake up.
#define LIGHT_SLEEP_DELAY_MARGIN 1500 //us


#define COWLORAPORT 3
#define TRACTORLORAPORT 4

#define LORAPORT COWLORAPORT

#define TIME_BETWEEN_STATE_SEND ((int64_t) 150) // *2s --- Time interval for sampling the state accumulator and sending the states

#endif //__AFARCLOUD_CONFIG
