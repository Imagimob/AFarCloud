/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// Configures LMIC and sets up the LoRa communication

#ifndef __afarcloud_lora_h
#define __afarcloud_lora_h

#include <Arduino.h>
#include "definitions.h"
#include "config.h"
#include <lmic.h>
#include "tbeam.h"

/****************
 * 
 *  Definitions for the paylaod buffer and accompanying logic
 * 
 ************/

//This needs to be set to true before sending operation, and will go back to false when the sending operation is done
//It is used to determine when the board is allowed to go into low power mode vs when it needs to stay powered for the LoRa operations to proceed
//In some distant future, we may build that into LMIC itself :)
extern bool LORA_sendjob_pending;

// Payload buffer.
// Vanilla LMIC allows max 51 bytes at every SF.
// It seems possible to remove that limitation quite easily, but for now we'll stick to it.
// https://www.thethingsnetwork.org/forum/t/lmic-fails-to-send-application-payload-larger-than-51-bytes/8923/19
extern byte LORA_payload[MAX_PAYLOAD];
extern int LORA_payload_length;
extern int LORA_port; //Effectively, one extra byte to be used for our application ;)
extern int LORA_ACK;  //0 for simple send, 1 for acknowledged sent.
// NB: acknowledged sent also lead to automatic adjustment of SF and power level.
// It's good to have them from time to time, but they consume ressources and airtime of the LoRa gateway.

extern bool LORA_payload_full;
extern int waypoints_in_payload;

// Reset the payload buffer
inline void reset_LoRa_payload()
{
    LORA_payload_full = false;
    LORA_payload_length = 0;
    LORA_ACK = 0;
    //LORA_port = 1;
    waypoints_in_payload = 0;
}

// Call this during initializing to configure everything
// NB: this will perform one attempt to connect to the LoRa network and block on failure.
// The board needs to be manually reset before another attempt.
void setup_LoRa();

// This attemps to trigger a send if a payload is available, while enforcing airtime rules.
// Returns True if the send was triggered and False if it needs to be retried later.
// It needs to be called regularly in the main loop
bool LORA_try_send();

/****************
 * 
 *  Configuration for LMIC
 * 
 ************/

void onEvent(ev_t ev);
void os_getArtEui(u1_t *buf);
void os_getDevEui(u1_t *buf);
void os_getDevKey(u1_t *buf);

// In the original LMIC code, these config values were defined on the
// gcc commandline. Since Arduino does not allow easily modifying the
// compiler commandline, use this file instead.

//#define CFG_eu868 1
//#define CFG_us915 1
// This is the SX1272/SX1273 radio, which is also used on the HopeRF
// RFM92 boards.
//#define CFG_sx1272_radio 1
// This is the SX1276/SX1277/SX1278/SX1279 radio, which is also used on
// the HopeRF RFM95 boards.
#define CFG_sx1276_radio 1

// 16 μs per tick
// LMIC requires ticks to be 15.5μs - 100 μs long
#define US_PER_OSTICK_EXPONENT 4
#define US_PER_OSTICK (1 << US_PER_OSTICK_EXPONENT)
#define OSTICKS_PER_SEC (1000000 / US_PER_OSTICK)

// Set this to 1 to enable some basic debug output (using printf) about
// RF settings used during transmission and reception. Set to 2 to
// enable more verbose output. Make sure that printf is actually
// configured (e.g. on AVR it is not by default), otherwise using it can
// cause crashing.
#define LMIC_DEBUG_LEVEL 0

// Enable this to allow using printf() to print to the given serial port
// (or any other Print object). This can be easy for debugging. The
// current implementation only works on AVR, though.
//#define LMIC_PRINTF_TO Serial

// Any runtime assertion failures are printed to this serial port (or
// any other Print object). If this is unset, any failures just silently
// halt execution.
#define LMIC_FAILURE_TO Serial

// Uncomment this to disable all code related to joining
//#define DISABLE_JOIN
// Uncomment this to disable all code related to ping
//#define DISABLE_PING
// Uncomment this to disable all code related to beacon tracking.
// Requires ping to be disabled too
//#define DISABLE_BEACONS

// Uncomment these to disable the corresponding MAC commands.
// Class A
//#define DISABLE_MCMD_DCAP_REQ // duty cycle cap
//#define DISABLE_MCMD_DN2P_SET // 2nd DN window param
//#define DISABLE_MCMD_SNCH_REQ // set new channel
// Class B
//#define DISABLE_MCMD_PING_SET // set ping freq, automatically disabled by DISABLE_PING
//#define DISABLE_MCMD_BCNI_ANS // next beacon start, automatical disabled by DISABLE_BEACON

// In LoRaWAN, a gateway applies I/Q inversion on TX, and nodes do the
// same on RX. This ensures that gateways can talk to nodes and vice
// versa, but gateways will not hear other gateways and nodes will not
// hear other nodes. By uncommenting this macro, this inversion is
// disabled and this node can hear other nodes. If two nodes both have
// this macro set, they can talk to each other (but they can no longer
// hear gateways). This should probably only be used when debugging
// and/or when talking to the radio directly (e.g. like in the "raw"
// example).
//#define DISABLE_INVERT_IQ_ON_RX

// This allows choosing between multiple included AES implementations.
// Make sure exactly one of these is uncommented.
//
// This selects the original AES implementation included LMIC. This
// implementation is optimized for speed on 32-bit processors using
// fairly big lookup tables, but it takes up big amounts of flash on the
// AVR architecture.
// #define USE_ORIGINAL_AES
//
// This selects the AES implementation written by Ideetroon for their
// own LoRaWAN library. It also uses lookup tables, but smaller
// byte-oriented ones, making it use a lot less flash space (but it is
// also about twice as slow as the original).
#define USE_IDEETRON_AES

#endif __afarcloud_lora_h
