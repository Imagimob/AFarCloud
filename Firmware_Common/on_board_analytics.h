/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#ifndef __afarcloud__onboard_analytics_h
#define __afarcloud__onboard_analytics_h

/***************************************************************
 *
 *          Various routines for processng of
 *                 GPS fixes on device
 *
 ***************************************************************/

#include <Arduino.h>
#include "definitions.h"
#include "config.h"
#include "ring_buffer.h"

/// Calculate distance in meters between two fixes
//  http://www.movable-type.co.uk/scripts/latlong.html
inline double getDistanceBetweenFixes(Fix *fix1, Fix *fix2)
{

    double dLat = deg2rad(fix1->latitude - fix2->latitude);
    double dLon = deg2rad(fix1->latitude - fix2->latitude);
    double t1 = sin(dLat / 2);
    double t2 = sin(dLon / 2);
    double a = t1 * t1 + cos(deg2rad(fix1->latitude)) * cos(deg2rad(fix2->latitude)) * t2 * t2;
    return R * 2 * atan2(sqrt(a), sqrt(1 - a));
}

/// Calculate time between two fixes (in seconds)
inline int getTimeBetweenFixes(Fix *fix1, Fix *fix2)
{
    int deltat = fix2->timestamp - fix1->timestamp;
    if (deltat < 0)
        deltat = -deltat;
    return deltat;
}

// Run Douglas Peucker on fixes in given interval and classify them as corners (or not corners)
void douglasPeuckerClassify(unsigned int from, unsigned int to, unsigned int DPC_level);

//Run Douglas-Peucker on the entire buffer
inline void douglasPeuckerClassify()
{
    douglasPeuckerClassify(oldestIndex, newestIndex, 1);
#ifdef DEBUG
    dumpBuffer();
#endif
}

#endif //__afarcloud__onboard_analytics_h
