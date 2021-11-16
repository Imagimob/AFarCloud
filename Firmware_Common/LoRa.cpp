/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

// Configures LMIC and sets up the LoRa communication


#include "LoRa.h"

//This needs to be set to true before sending operation, and will go back to false when the sending operation is done
//It is used to determine when the board is allowed to go into low power mode vs when it needs to stay powered for the LoRa operations to proceed
//In some distant future, we may build that into LMIC itself :)
bool LORA_sendjob_pending=false;

// Vanilla LMIC allows max 51 bytes at every SF.
// It seems possible to remove that limitation quite easily, but for now we'll stick to it.
// https://www.thethingsnetwork.org/forum/t/lmic-fails-to-send-application-payload-larger-than-51-bytes/8923/19
byte LORA_payload[MAX_PAYLOAD];
int LORA_payload_length = 0;
int LORA_port=0;  //one extra byte to be used ;)
int LORA_ACK = 0; //0 for simple send, 1 for acknowledged sent
bool LORA_payload_full = false;
bool LORA_try_send();
int waypoints_in_payload = 0;



//The LORA send function
bool LORA_try_send(){

    if (LMIC.opmode & OP_TXRXPEND) return false;

    #ifdef DEBUG
    Serial.print("Sending... Payload length: ");
    Serial.println(LORA_payload_length);
    #endif

    // Prepare upstream data transmission at the next possible time.
    LORA_sendjob_pending=true;
    LMIC_setTxData2(LORA_port, LORA_payload, LORA_payload_length, LORA_ACK);

    //blue_led_on();
    #ifdef DEBUG
    Serial.println("Send request done.");
    #endif
    
    reset_LoRa_payload();

    return true;
}


//Callback functions for LMIC
void inline tx_complete(){
    #ifdef DEBUG
    Serial.println("****************** YOUPEEEEEE ***********");
    #endif
    LORA_sendjob_pending=false;};
void inline joining(){};
void inline joined(){blue_led_off();};




/* #####################################################################
 * #
 * #
 * #        LMIC part
 * #
 * #
 * #
 * #####################################################################
 */

#ifdef DEBUG
void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            joining();
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            joined();
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
                Serial.println(F("Received ack"));
            if (LMIC.dataLen) {
                Serial.println(F("Received "));
                Serial.println(LMIC.dataLen);
                Serial.println(F(" bytes of payload"));
            }
            tx_complete();
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        default:
            Serial.println(F("Unknown event"));
            break;
    }
}
#else

void onEvent (ev_t ev) {
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            break;
        case EV_BEACON_FOUND:
            break;
        case EV_BEACON_MISSED:
            break;
        case EV_BEACON_TRACKED:
            break;
        case EV_JOINING:
            joining();
            break;
        case EV_JOINED:
            // Disable link check validation (automatically enabled
            // during join, but not supported by TTN at this time).
            LMIC_setLinkCheckMode(0);
            joined();
            break;
        case EV_RFU1:
            break;
        case EV_JOIN_FAILED:
            break;
        case EV_REJOIN_FAILED:
            break;
        case EV_TXCOMPLETE:
            //if (LMIC.txrxFlags & TXRX_ACK)
            //if (LMIC.dataLen)
            tx_complete();
            break;
        case EV_LOST_TSYNC:
            break;
        case EV_RESET:
            break;
        case EV_RXCOMPLETE:
            break;
        case EV_LINK_DEAD:
            break;
        case EV_LINK_ALIVE:
            break;
        default:
            break;
    }
}

#endif //DEBUG


// EUI
static const u1_t PROGMEM APPEUI[8] = APP_EUI;
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }

// DEVEUI
static const u1_t PROGMEM DEVEUI[8] = DEV_EUI;
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }

// APPKEY
static const u1_t PROGMEM APPKEY[16] = APP_KEY;
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }



void setup_LoRa() {
    #ifdef DEBUG
    Serial.println("Initializing LoRa module");
    #endif //DEBUG
    // LMIC init
    os_init();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();


#if defined(CFG_eu868)
    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set.
    // NA-US channels 0-71 are configured automatically
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);      // g2-band
    // TTN defines an additional channel at 869.525Mhz using SF9 for class B
    // devices' ping slots. LMIC does not have an easy way to define set this
    // frequency and support for class B is spotty and untested, so this
    // frequency is not configured here.
#elif defined(CFG_us915)
    // NA-US channels 0-71 are configured automatically
    // but only one group of 8 should (a subband) should be active
    // TTN recommends the second sub band, 1 in a zero based count.
    // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
    LMIC_selectSubBand(1);
#endif

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DOWNLINK_DATA_RATE;

    pinMode(BUILTIN_LED, OUTPUT);
    blue_led_off();

#ifdef DEBUG
    Serial.println("Sending first message \n");
#endif //DEBUG
    //Quick and dirty hack to send something immediately upon reset
    if (LMIC.opmode & OP_TXRXPEND) {
#ifdef DEBUG
        Serial.println(F("OP_TXRXPEND, not sending"));
#endif
    } else {
        // Prepare upstream data transmission at the next possible time.
        LORA_sendjob_pending=true;
        LMIC_setTxData2(0, LORA_payload, 1, 0);

        // Set data rate and transmit power for uplink
        // This needs to be run AFTER the first send is initialized, otherwise it is ignored.
        LMIC_setDrTxpow(UPLINK_DATA_RATE, UPLINK_POWER_LEVEL);

        while (LORA_sendjob_pending) {
            os_runloop_once();
            //esp_sleep_enable_timer_wakeup(500);
            //int ret = esp_light_sleep_start();
        }
    }


}


//##########################  Enf od LMIC part #######################


