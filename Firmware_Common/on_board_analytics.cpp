/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "on_board_analytics.h"

/***************************************************************
 *
 *          Various routines for processng of
 *                 GPS fixes on device
 *
 ***************************************************************/

// Calculate perpendicular distance between a fix and a line
// https://stackoverflow.com/questions/7803004/distance-from-point-to-line-on-earth
double getDistanceBetweenFixAndLine(int p, int from, int to)
{
    double d0 = getTimeBetweenFixes(&fixes[p], &fixes[from]);
    double d1 = getTimeBetweenFixes(&fixes[from], &fixes[to]);
    double d2 = getTimeBetweenFixes(&fixes[to], &fixes[p]);

    // Compute triangle area
    double halfPerimeter = (d0 + d1 + d2) * 0.5;
    double triangleArea = sqrt(halfPerimeter * (halfPerimeter - d0) * (halfPerimeter - d1) * (halfPerimeter - d2));
    //SerialUSB.println("Triangle Area: " + String(triangleArea,2));
    // Compute perpendicular distance
    return 2 * triangleArea / d1;
}

// Run Douglas Peucker on fixes in given interval and classify them as corners (or not corners)
void douglasPeuckerClassify(unsigned int from, unsigned int to, unsigned int DPC_level)
{
    if (from == to)
        return; //No datapoints added since previous call to this function

    //SerialUSB.println("RDP from: " + String(from) + " to: " + String(to));
    double distMax = 0;
    int distMaxIndex = 0;
    // Mark first and last as corner
    //Serial.println("Marking " + String(from) + " as corner");
    fixes[from].corner = true;
    fixes[from].corner_order = DPC_level;

    //Serial.println("Marking " + String(to) + " as corner");
    fixes[to].corner = true;
    fixes[to].corner_order = DPC_level;

    // For all inbetween
    gotoIndex(from);
    gotoNext();
    if (cursorIndex == to)
    {
        // There are no fixes inbetween
        //Serial.println("No fixes between!");
        return;
    }
    while (cursorIndex != to)
    {
        //Serial.println("Calculating distance for " + String(cursorIndex));
        double dist = getDistanceBetweenFixAndLine(cursorIndex, from, to);
        if (dist > distMax)
        {
            distMaxIndex = cursorIndex;
            distMax = dist;
        }
        gotoNext();
    }
    if (distMax > EPSILON)
    {
        // Recurse by split at distMaxIndex
        douglasPeuckerClassify(from, distMaxIndex, DPC_level + 1);
        douglasPeuckerClassify(distMaxIndex, to, DPC_level + 1);
    }
    else
    {
        // Mark all points in between as non corners
        gotoIndex(from);
        gotoNext();
        while (cursorIndex != to)
        {
            //Serial.println("Marking " + String(cursorIndex) + " as non corner");
            fixes[cursorIndex].corner = false;
            gotoNext();
        }
    }
}
