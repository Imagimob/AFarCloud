/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#ifndef __afarcloud_ringbuffer
#define __afarcloud_ringbuffer

#include <Arduino.h>
#include "definitions.h"
#include "config.h"

enum BufferStatus {BUFFER_OK, BUFFER_EMPTY, BUFFER_FULL};


/***************************************************************
 *
 *          A simple ring buffer for GPS fixes
 *
 *
 *
 ***************************************************************/

extern unsigned int oldestIndex; // Where the oldest fix is (unless == newestIndex, meaning empty)
extern unsigned int newestIndex; // Where we stored the latest fix pushed to the buffer
extern unsigned int cursorIndex; // Used during buffer iteration

extern Fix fixes[BUFFER_SIZE];


// Push a new fix last in buffer
enum BufferStatus push(Fix &fix);

// Pop first fix in buffer --- never used...
enum BufferStatus pop(Fix &fix);

// Push a fix even if buffer is full, thus also removing the oldest
void forcePush(Fix &fix);

// Iteration support, all return true for success
bool gotoIndex(unsigned int index);

bool bufferEmpty();
unsigned int getFirstIndex();
unsigned int getLastIndex();

bool gotoLast();
bool gotoFirst();

//TBA: check if there is any case where the gated version doesn't work instead of the non-gated one.
bool gotoNext();
bool gotoNextGated();
bool gotoPrevious();
bool readFix(Fix &fix);
bool setOldestIndex();
bool mark_as_sent(int index);

#ifdef DEBUG
// Print out a single fix
void dumpFix(Fix &fix);

// Print out a single fix
void dumpFix2(Fix &fix);

// Print out current buffer in full, for test purposes
void dumpBuffer();
#endif //DEBUG



#endif __afarcloud_ringbuffer