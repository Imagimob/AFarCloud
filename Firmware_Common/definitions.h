/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

/***************************************************************
 *
 * 
 *        Definitions and constants used throughout the codebase
 *
 *
 ***************************************************************/

#ifndef __AFARCLOUD_DEFINITIONS
#define __AFARCLOUD_DEFINITIONS
#include <Arduino.h>
#include "config.h"

#include <stdint.h>
#define byte uint8_t

#define MAX_PAYLOAD 51   // We use SF10 which in Europe has 59 bytes max --- but Lmic supports max 51
#define MAX_IN_PAYLOAD 5 // 16 + 4*8 = 48 bytes which fits in SF10

#if !defined(__time_t_defined) // avoid conflict with newlib or other posix libc
typedef unsigned long time_t;
#define __time_t_defined
#endif

const double R = 6371000.8; // Radius of the earth in m

//From Time lib
#define MICROSECOND_PER_SECONDS ((int)1000000)
#define SECS_PER_MIN ((time_t)(60UL))
#define SECS_PER_HOUR ((time_t)(3600UL))
#define SECS_PER_DAY ((time_t)(SECS_PER_HOUR * 24UL))
#define DAYS_PER_WEEK ((time_t)(7UL))
#define SECS_PER_WEEK ((time_t)(SECS_PER_DAY * DAYS_PER_WEEK))
#define SECS_PER_YEAR ((time_t)(SECS_PER_DAY * 365UL)) // TODO: ought to handle leap years
#define SECS_YR_2000 ((time_t)(946684800UL))           // the time at the start of y2k
// leap year calulator expects year argument as years offset from 1970
#define LEAP_YEAR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))
#define PI 3.1415926535897932384626433832795

typedef struct
{
  uint8_t Second;
  uint8_t Minute;
  uint8_t Hour;
  uint8_t Wday; // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month;
  uint8_t Year; // offset from 1970;
} tmElements_t;

// A struct representing a single GPS fix
typedef struct
{
  unsigned long timestamp;
  double latitude;
  double longitude;
  int altitude;
  byte speed;
  int Nsats;
  float hdop;
  bool corner = false;
  unsigned int corner_order = 0;
  //int frame = 0; //I think this is not used
  bool sent = false; //Sent here means "it was encoded and a sent attempt was done"

#ifdef BME280
  float temperature;
  float humidity;
  float pressure;
  float alt_barometric;
#endif
  float battery_voltage;
} Fix;

/***************************************************************
 *
 *         Some utility functions of general usefulnes
 *
 *
 ***************************************************************/

time_t inline UnixTimeStamp(const tmElements_t &tm)
{
  // assemble time elements into time_t
  // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
  // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9
  static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // API starts months from 1, this array starts from 0

  int i;
  uint32_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds = tm.Year * (SECS_PER_DAY * 365);
  for (i = 0; i < tm.Year; i++)
  {
    if (LEAP_YEAR(i))
    {
      seconds += SECS_PER_DAY; // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm.Month; i++)
  {
    if ((i == 2) && LEAP_YEAR(tm.Year))
    {
      seconds += SECS_PER_DAY * 29;
    }
    else
    {
      seconds += SECS_PER_DAY * monthDays[i - 1]; //monthDay array starts from 0
    }
  }
  seconds += (tm.Day - 1) * SECS_PER_DAY;
  seconds += tm.Hour * SECS_PER_HOUR;
  seconds += tm.Minute * SECS_PER_MIN;
  seconds += tm.Second;
  return (time_t)seconds;
}

double inline deg2rad(double deg)
{
  return deg * (PI / 180);
}

#endif // #__AFARCLOUD_DEFINITIONS
