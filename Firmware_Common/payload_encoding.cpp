/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "payload_encoding.h"
/* #####################################################################
 * #
 * #
 * #        Payload encoding logic
 * #
 * #
 * #
 * #####################################################################
 */



unsigned long int previousTimestamp = 0;
double previousLatitude = 0;
double previousLongitude = 0;
int previousAltitude = 0;


#include "accel.h"

inline unsigned int encode_battery(float battery_voltage){
    //Battery should be ~3-4.5 --- let's go for 2.56-5.12V
    return ( (unsigned int) (battery_voltage*100))-256;
}

int64_t LastStateSent =0 ;

inline int encodeStates(){
    const float accumulated_t = getAccumulatorTime()*1.0;
    for (int i=1;i<IMAI_DATA_OUT_COUNT;i++) //We skip the first default state. It'll be reconstructed at the receiver. 
    {
        const float val = (255.0*getAccumulatorValue(i))/accumulated_t;
        const byte tmp =  (byte) max(0, min(255, (int)floor(val))); 

        #ifdef DEBUG
        Serial.print("State ");
        Serial.print(i);
        Serial.print(" : ");
        Serial.print(val);
        Serial.print(" ---- ");
        Serial.println(tmp);
        #endif
        LORA_payload[LORA_payload_length++]= tmp;
  
    }
    resetAccumulator();
    LastStateSent = esp_timer_get_time();
}

// Add a given fix to current payload, returns false if encoding failed due to payload size overrun
bool encodeFix(Fix &fix) {
    long lat = round(fix.latitude * 1000000);
    long lon = round(fix.longitude * 1000000);
    bool incremental = false;

    // Can we make an incremental waypoint?
    if (waypoints_in_payload != 0) {
        if (fix.timestamp < previousTimestamp) {
            //Serial.println("SHOULD NOT HAPPEN! WRONG TIMESTAMP! Fix:" + String(fix.timestamp) + " previous: " + String(previousTimestamp));
            incremental = false; //Do a full waypoint if this happens. Will happen when we used all corners and start to push non-corner points
        }
        // Is gap small enough? If too large gap in time, altitude or distance, we will send it in full instead.
        incremental = ((fix.timestamp - previousTimestamp) < 255) &&
                      (abs(lat - previousLatitude) < 32767) &&
                      (abs(lon - previousLongitude) < 32767) &&
                      (abs(fix.altitude - previousAltitude) < 127);
    }
    // Create full or incremental waypoint payload
    if (!incremental) {
        // This fix is full, 16 bytes.
        // Did we run out of space in payload?
        if (MAX_PAYLOAD < (LORA_payload_length + 16)) {
            return false;
        }
        //Serial.println("Full waypoint ");
        //Serial.println("latitude = " + String(fix.latitude, 6));
        //Serial.println("longitude = " + String(fix.longitude, 6));
        byte flags = FULL_FIX;
        if (fix.corner) {
            flags |= CORNER;
        }

        LORA_payload[LORA_payload_length++] = flags;
        LORA_payload[LORA_payload_length++] = b1(fix.timestamp);
        LORA_payload[LORA_payload_length++] = b2(fix.timestamp);
        LORA_payload[LORA_payload_length++] = b3(fix.timestamp);
        LORA_payload[LORA_payload_length++] = b4(fix.timestamp);
        LORA_payload[LORA_payload_length++] = b1(lat);
        LORA_payload[LORA_payload_length++] = b2(lat);
        LORA_payload[LORA_payload_length++] = b3(lat);
        LORA_payload[LORA_payload_length++] = b4(lat);
        LORA_payload[LORA_payload_length++] = b1(lon);
        LORA_payload[LORA_payload_length++] = b2(lon);
        LORA_payload[LORA_payload_length++] = b3(lon);
        LORA_payload[LORA_payload_length++] = b4(lon);
        LORA_payload[LORA_payload_length++] = b1(fix.altitude);
        LORA_payload[LORA_payload_length++] = b2(fix.altitude);
        LORA_payload[LORA_payload_length++] = fix.speed;

        previousTimestamp = fix.timestamp;
        previousLatitude = lat;
        previousLongitude = lon;
        previousAltitude = fix.altitude;
        waypoints_in_payload++;

    } else {
        // This fix is incremental, 8 bytes
        // Did we run out of space in payload?
        if (MAX_PAYLOAD < (LORA_payload_length + 8)) {
            //sendBuf_full = true;
            return false;
        }
        //Serial.println("Incremental waypoint");
        //Serial.println("latitude = " + String(fix.latitude, 6));
        //Serial.println("longitude = " + String(fix.longitude, 6));
        byte diffSeconds = byte(fix.timestamp - previousTimestamp);
        int diffLatitude = int(lat - previousLatitude);
        int diffLongitude = int(lon - previousLongitude);
        byte diffAltitude = byte(fix.altitude - previousAltitude);
        byte flags = ~FULL_FIX;
        if (fix.corner) {
            flags |= CORNER;
        }
        LORA_payload[LORA_payload_length++] = flags;
        LORA_payload[LORA_payload_length++] = diffSeconds;
        LORA_payload[LORA_payload_length++] = b1(diffLatitude);
        LORA_payload[LORA_payload_length++] = b2(diffLatitude);
        LORA_payload[LORA_payload_length++] = b1(diffLongitude);
        LORA_payload[LORA_payload_length++] = b2(diffLongitude);
        LORA_payload[LORA_payload_length++] = diffAltitude;
        LORA_payload[LORA_payload_length++] = fix.speed;
        previousTimestamp = fix.timestamp;
        previousLatitude = lat;
        previousLongitude = lon;
        previousAltitude = fix.altitude;
        waypoints_in_payload++;
    }

    return true;
}

unsigned int corners_in_payload[MAX_IN_PAYLOAD+1];
int corners_in_payload_idx=0;

void mark_sent_fixes() {
    Fix tmp_fix;

//    Serial.println("Marking fixes as sent");
    for (int i = 0; i < corners_in_payload_idx; i++) {

  //      Serial.print("Cursor: ");
  //  Serial.println(corners_in_payload[i]);
        mark_as_sent(corners_in_payload[i]);

        //dumpBuffer();
        //setOldestIndex();
    }
}

bool SendNeeded = false;


// Add a given fix to current payload, returns false if encoding failed due to payload size overrun
void build_payload_and_send(Fix *last_fix) {
    if (LORA_sendjob_pending){
        //If we have a pending send, we do nothing. 
        //Serial.println("Pending LORA send");
        return;
    }

   if (SendNeeded) {
        if (LORA_try_send()) 
        {
            SendNeeded = false;
            return;
        }
    }

    // Build payload
    Fix tmp_fix;
    reset_LoRa_payload();

    #ifdef DEBUG
    Serial.println("encoded battery");
    Serial.println(b1(encode_battery(getBatteryVoltage())));
    #endif

    LORA_port = LORAPORT;
    LORA_payload[LORA_payload_length++]=b1(encode_battery(getBatteryVoltage()));

    corners_in_payload_idx = 0;

    if  ((esp_timer_get_time()-LastStateSent) > (TIME_BETWEEN_STATE_SEND*MICROSECOND_PER_SECONDS))
    {
        #ifdef DEBUG
        Serial.println("About to send");
        #endif

        // Here, we send a state + the last GPS fix if available
        encodeStates();
        SendNeeded = true;

        if (last_fix!=NULL) encodeFix(*last_fix);
        else {
            if (gotoLast()) {
                readFix(tmp_fix);
                encodeFix(tmp_fix);
                mark_as_sent(cursorIndex);
            }
            else {
                //We haven't got our first GPS fix yet. Defaulting to an all-zero fix.
                Fix defaultFix; 
                defaultFix.timestamp      = 0;
                defaultFix.latitude       = 0;
                defaultFix.longitude      = 0;
                defaultFix.altitude       = 0;
                defaultFix.speed          = 0;
                defaultFix.Nsats          = 0;
                defaultFix.hdop           = 0;
                defaultFix.corner         = 0;
                defaultFix.battery_voltage= 0;

                #ifdef BME280
                defaultFix.temperature    = 0;
                defaultFix.humidity       = 0;
                defaultFix.pressure       = 0;
                defaultFix.alt_barometric = 0;
                #endif 
                
                #ifdef DEBUG
                Serial.println("Sending default fix");
                encodeFix(defaultFix);
                #endif
            }
        }
        return;
    }
    else
    {
        // For the time being, we do nothing here. When the above will work fine, we'll come back to it.
        //gotoFirst();

        return;
    }

    #ifdef DEBUG
    Serial.print("Length of payload: ");
    Serial.println(LORA_payload_length);
    #endif

    if (SendNeeded) {
        if (LORA_try_send()) 
        {
            SendNeeded = false;
            return;
        }
    }


    if (bufferEmpty()) {
        //Serial.println("Nothing to send");
        return;
    }

    

    unsigned int corners2encode[BUFFER_SIZE];
    int count2encode = 0;

    //Scan for non sent corners
    do {
        if (!readFix(tmp_fix))
            break;
        if ((!tmp_fix.sent) && (tmp_fix.corner)) {

/*
            Serial.print("Selecting corner at position ");
            Serial.print(count2encode);
            Serial.print(",");
            Serial.print(cursorIndex);
*/
            corners2encode[count2encode++] = cursorIndex;

        }

    } while (gotoNextGated());


//    Serial.print("count2encode : ");
//    Serial.println(count2encode);
    for (int i = 0; i < count2encode; i++) {
/*
        Serial.print("Trying to encode fix at position ");
        Serial.print(i);
        Serial.print(",");
        Serial.println(corners2encode[i]);
*/
        cursorIndex = corners2encode[i];
        readFix(tmp_fix);

        if (encodeFix(tmp_fix)) {
            //Serial.println("Succeeded");
            corners_in_payload[corners_in_payload_idx++] = corners2encode[i];
        } else {
            //Serial.println("Lora buffer full --- we should send after this :-/");
            LORA_payload_full = true;
            break;
        };
        //Serial.print("LoRa payload lenght after: ");
        //Serial.println(LORA_payload_length);
    }
  
    
    //TBA: send non-corners if some payload space left
}


// ######################### Payload encoding #######################
