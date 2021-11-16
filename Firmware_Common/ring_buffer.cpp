/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "ring_buffer.h"

/***************************************************************
 *
 *         A simple ring buffer for GPS fixes
 *
 *
 *
 ***************************************************************/

unsigned int oldestIndex = 0; // Where the oldest fix is (unless == newestIndex, meaning empty)
unsigned int newestIndex = 0; // Where we stored the latest fix pushed to the buffer
unsigned int cursorIndex;
Fix fixes[BUFFER_SIZE];

// Push a new fix last in buffer
enum BufferStatus push(Fix &fix)
{

    unsigned int nextIndex = (newestIndex + 1) % BUFFER_SIZE;
    if (nextIndex == oldestIndex)
    {
        return BUFFER_FULL;
    }
    fixes[newestIndex] = fix;

    newestIndex = nextIndex;

#ifdef DEBUG
    Serial.print("Pushing - buffer state after push: ");
    Serial.print(newestIndex);
    Serial.print(",");
    Serial.println(oldestIndex);
#endif //DEBUG

    return BUFFER_OK;
}

// Pop first fix in buffer --- never used in this code
/*enum BufferStatus pop(Fix &fix){
    if (newestIndex == oldestIndex){
        return BUFFER_EMPTY;
    }
    fix = fixes[oldestIndex];
    oldestIndex = (oldestIndex+1) % BUFFER_SIZE;
    return BUFFER_OK;
}
*/

// Push a fix even if buffer is full, thus also removing the oldest
void forcePush(Fix &fix)
{
    BufferStatus status = push(fix);
    if (status == BUFFER_FULL)
    {

#ifdef DEBUG
        Serial.println("Overwriting old value");
#endif //DEBUG

        // Throw away oldest and push again
        oldestIndex = (oldestIndex + 1) % BUFFER_SIZE;
        push(fix);
    }
}

// Iteration support, all return true for success
bool gotoIndex(unsigned int index)
{
    cursorIndex = index % BUFFER_SIZE;
    return true;
}
bool bufferEmpty()
{
    return (newestIndex == oldestIndex);
}
unsigned int getFirstIndex()
{
    return oldestIndex;
}
unsigned int getLastIndex()
{
    return (newestIndex + BUFFER_SIZE - 1) % BUFFER_SIZE;
}
bool gotoLast()
{
    if (newestIndex == oldestIndex)
    {
        return false;
    }
    cursorIndex = (newestIndex + BUFFER_SIZE - 1) % BUFFER_SIZE;
    return true;
}
bool gotoFirst()
{
    if (newestIndex == oldestIndex)
    {
        return false;
    }
    cursorIndex = oldestIndex;
    return true;
}

//TBA: check if there is any case where the gated version doesn't work instead of the non-gated one.
bool gotoNext()
{
    if (newestIndex == oldestIndex)
    {
        return false;
    }
    cursorIndex = (cursorIndex + 1) % BUFFER_SIZE;
    return true;
}
bool gotoNextGated()
{
    /*
    Serial.println("RING BUFFER: Going to next ");
    Serial.print("RING BUFFER: State before ");
    Serial.print(oldestIndex);
    Serial.print(",");
    Serial.print(newestIndex);
    Serial.print(",");
    Serial.println(cursorIndex);
*/

    if (newestIndex == oldestIndex)
    {
        return false;
    }
    if (newestIndex == ((cursorIndex + 1) % BUFFER_SIZE))
        return false;

    cursorIndex = (cursorIndex + 1) % BUFFER_SIZE;

    //    Serial.print("RING BUFFER: State after ");
    //    Serial.println(cursorIndex);

    return true;
}

bool gotoPrevious()
{
    if (newestIndex == oldestIndex)
    {
        return false;
    }
    cursorIndex = (cursorIndex + BUFFER_SIZE - 1) % BUFFER_SIZE;
    return true;
}
bool readFix(Fix &fix)
{
    if (newestIndex == oldestIndex)
    {
        return false;
    }
    fix = fixes[cursorIndex];
    return true;
}

bool setOldestIndex()
{
    if (newestIndex == oldestIndex)
    {
        return false;
    }

    //TBA: add some check here...
    oldestIndex = cursorIndex;
    return true;
}

bool mark_as_sent(int index)
{
    fixes[index].sent = true;
}

//inline unsigned int linearize_index(unsigned int cursor){
//    return ( (cursor-oldestIndex+BUFFER_SIZE) % BUFFER_SIZE);
//}

//inline unsigned int fold_index()

#ifdef DEBUG
// Print out a single fix
void dumpFix(Fix &fix)
{
    Serial.println("FIX");
    Serial.println("Timestamp: " + String(fix.timestamp));
    Serial.println("Lat: " + String(fix.latitude, 8));
    Serial.println("Lon: " + String(fix.longitude, 8));
    Serial.println("Corner: " + String(fix.corner));
    Serial.println("Frame: " + String(fix.frame));
    Serial.println("Number satelites: " + String(fix.Nsats));
    Serial.println("HDOP: " + String(fix.hdop));
    Serial.println("--------------------");
}

// Print out a single fix
void dumpFix2(Fix &fix, unsigned int cursorIndex)
{
    Serial.print("Fix " + String(cursorIndex) + " : " + String(fix.timestamp));
    Serial.print("-(" + String(fix.latitude, 8));
    Serial.print("," + String(fix.longitude, 8) + ")");
    Serial.print("--- " + String(fix.corner));
    Serial.print(", " + String(fix.corner_order));
    Serial.println(", " + String(fix.sent));
}

// Print out current buffer in full, for test purposes
void dumpBuffer()
{
    Serial.println("Dumping buffer--------------------");
    if (gotoFirst())
    { // Not empty
        do
            dumpFix2(fixes[cursorIndex], cursorIndex);
        while (gotoNextGated());

        dumpFix2(fixes[cursorIndex + 1], cursorIndex + 1);
    }
    Serial.println("Dump done--------------------");
}
#endif //DEBUG
