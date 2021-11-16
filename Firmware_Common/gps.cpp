/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "gps.h"

/***************************************************************
 *
 *          Basic GPS functionalities
 *
 ***************************************************************/

// A helper object to decode NMEA protocol on the serial link to an easier to use position object
TinyGPSPlus tGps;

HardwareSerial GPSSerial(1);

// Initializes the GPS, getting the pins dynamically from the board configuration routines
void gps_initialize()
{
    GPSSerial.begin(9600, SERIAL_8N1, getGPSTXpin(), getGPSRXpin());
    GPSSerial.setTimeout(2);
}

// Reads what is available on the serial bus and pushes it to the TinyGPSPlus plus
void gps_pull_loop()
{
    while (GPSSerial.available())
    {
        char data = GPSSerial.read();
        tGps.encode(data);
    };
}

// This will block and pull the serial till a position is available
void gps_Update_and_GetFix(Fix *fix)
{
    do
    {
        gps_pull_loop();
    } while (!gps_getFix(fix));
#ifdef DEBUG
    Serial.println("Got updated fix!");
#endif
}

// Get the last fix into a Fix object
bool gps_getFix(Fix *fix)
{
    if ((tGps.time.isUpdated()) &
        (tGps.location.isUpdated()) &
        (tGps.altitude.isUpdated()) &
        (tGps.speed.isUpdated()) &
        (tGps.hdop.isUpdated()) &
        (tGps.satellites.isUpdated()))
    {

        tmElements_t tm;

        tm.Second = tGps.time.second();
        tm.Minute = tGps.time.minute();
        tm.Hour = tGps.time.hour();
        tm.Day = tGps.date.day();
        tm.Month = tGps.date.month();
        tm.Year = tGps.date.year() - 1970;

        fix->timestamp = UnixTimeStamp(tm);
        fix->latitude = tGps.location.lat();
        fix->longitude = tGps.location.lng();
        fix->altitude = tGps.altitude.meters();
        fix->speed = tGps.speed.kmph();
        fix->hdop = tGps.hdop.value();
        fix->Nsats = tGps.satellites.value();

        return true;
    }
    return false;
}

// Verify quality of the fix
bool checkGpsFix()
{
#ifdef DEBUG
    Serial.println("GPS stuff:");
    Serial.println(tGps.location.isValid());
    Serial.println(tGps.satellites.value());
    Serial.println(tGps.location.age());
    Serial.println(tGps.hdop.isValid());
    Serial.println(tGps.hdop.value());
    Serial.println(tGps.hdop.age());
    Serial.println(tGps.altitude.isValid());
    Serial.println(tGps.altitude.age());
#endif

    if (tGps.location.isValid() &&
        tGps.satellites.value() >= MIN_SATELLITES &&
        tGps.location.age() < MAX_GPS_AGE &&
        tGps.hdop.isValid() &&
        tGps.hdop.value() <= MAX_HDOP &&
        tGps.hdop.age() < MAX_GPS_AGE &&
        tGps.altitude.isValid() &&
        tGps.altitude.age() < MAX_GPS_AGE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#ifdef DEBUG
// Print out a single fix
void printFix_full(Fix *fix)
{
    Serial.println("FIX");
    Serial.println("Timestamp: " + String(fix->timestamp));
    Serial.println("Lat: " + String(fix->latitude, 8));
    Serial.println("Lon: " + String(fix->longitude, 8));
    Serial.println("Speed: " + String(fix->speed) + " km/h");
    Serial.println("Corner: " + String(fix->corner));
    Serial.println("Frame: " + String(fix->frame));
    Serial.println("Number satelites: " + String(fix->Nsats));
    Serial.println("HDOP: " + String(fix->hdop));
    Serial.println("--------------------");
    Serial.println("Temperature: " + String(fix->temperature) + "°C");
    Serial.println("Humidity: " + String(fix->humidity) + "%");
    Serial.println("Pressure: " + String(fix->pressure) + " hPa");
    Serial.println("Battery voltage: " + String(fix->battery_voltage) + " V");
}

// Print out a single fix
void printFix(Fix *fix)
{
    Serial.print("Fix: " + String(fix->timestamp));
    Serial.print("-(" + String(fix->latitude, 8));
    Serial.print("," + String(fix->longitude, 8) + ")");
    Serial.print("--- " + String(fix->corner));
    Serial.println(", " + String(fix->sent));
}
#endif
