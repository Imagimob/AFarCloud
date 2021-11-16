/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#ifndef __afarcloud_payload_encoding
#define __afarcloud_payload_encoding


/* #####################################################################
 * #
 * #
 * #        Payload encoding logic
 * #
 * #
 * #
 * #####################################################################
 */

#include "Arduino.h"
#include "definitions.h"
#include "config.h"
#include "LoRa.h"
#include "ring_buffer.h"
#include "tbeam.h"
#include "accel.h"

#define FULL_FIX 0x1 
#define CORNER 0x2

// Used for byte extraction into payload buffer for encoding
#define b4(x) ((x >> 24) & 0xFF)
#define b3(x) ((x >> 16) & 0xFF)
#define b2(x) ((x >> 8) & 0xFF)
#define b1(x) (x & 0xFF)


// Add a given fix to current payload, returns false if encoding failed due to payload size overrun
bool encodeFix(Fix &fix);

void mark_sent_fixes();

// Add a given fix to current payload, returns false if encoding failed due to payload size overrun
void build_payload_and_send(Fix *);

void buildSendStatePayload();

#endif //__afarcloud_payload_encoding
