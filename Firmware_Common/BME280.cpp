/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "BME280.h"

Adafruit_BME280 bme(I2C_SDA, I2C_SCL); // these pins are defined in header file

bool BME280_status; // status after reading from BME280
float sea_level_pressure;

void initBME280()
{
    BME280_status = bme.begin();

#ifdef DEBUGBME280
    if (!BME280_status)
    {
        Serial.println(F("Could not find a valid BME280 sensor, check wiring!"));
    }
    else
    {
        Serial.println(F("BME280 initialized sucessfully"));
        delay(1000); // stabilize sensor readings
    }
#endif

    if (BME280_status)
    {
#ifdef DEBUGBME280
        Serial.println(F("BMP/BME280: FORCED mode, 16x pressure / 2x temperature / 1x humidity oversampling,"));
        Serial.println(F("0.5ms standby period, filter 16x"));
#endif

        bme.setSampling(Adafruit_BME280::MODE_FORCED,
                        Adafruit_BME280::SAMPLING_X2,  // temperature
                        Adafruit_BME280::SAMPLING_X16, // pressure
                        Adafruit_BME280::SAMPLING_X1,  // humidity
                        Adafruit_BME280::FILTER_X16,
                        Adafruit_BME280::STANDBY_MS_0_5);
    }
    update_sea_level_pressure(DEFAULT_SEA_LEVEL_PRESSURE_HPA);
}

void update_sea_level_pressure(float pressure)
{
    if ((pressure < 1100) & (pressure > 850))
        sea_level_pressure = pressure;
}

void takeBME280_measurement_async()
{
    bme.takeForcedMeasurementAsync();
}

bool isReadyBME280_measurement_async()
{
    bme.waitForcedMeasurementCompleted();
}

void waitBME280_measurement_ready()
{
#ifdef DEBUGBME280
    int64_t before = esp_timer_get_time();
#endif

    bme.waitForcedMeasurementCompleted();

#ifdef DEBUGBME280
    int64_t after = esp_timer_get_time();
    Serial.print("Waited ");
    Serial.print(after - before);
    Serial.println(" us for BME280 measurement");
#endif
}

void getBME280Values_async(float *temp, float *pressure, float *alt_barometric, float *hum)
{
    //Careful - if this is called without isReadyBME280_measurement_async==true, it will return old values!
    //If you aren't sure what to do, you probably want to run getBME280Values();

    if (!BME280_status)
    { // we don't have BME280 connection, clear the values and exit
        *temp = 0.0f;
        *pressure = 0.0f;
        *alt_barometric = 0.0f;
        *hum = 0.0f;
        return;
    }

    *temp = bme.readTemperature();
    *pressure = bme.readPressure() / 100.0F;
    *alt_barometric = bme.readAltitude(sea_level_pressure);
    *hum = bme.readHumidity();

#ifdef DEBUGBME280
    Serial.print(F("Temperature = "));
    Serial.print(*temp);
    Serial.print("C, ");
    Serial.print("Pressure = ");
    Serial.print(*pressure);
    Serial.print("hPa, ");
    Serial.print("Approx. Altitude = ");
    Serial.print(*alt_barometric);
    Serial.print("m, ");
    Serial.print("Humidity = ");
    Serial.print(*hum);
    Serial.println("%");

    delay(100);
#endif
}

void getBME280Values(float *temp, float *pressure, float *alt_barometric, float *hum)
{

    if (BME280_status)
        takeBME280_measurement_async();

    waitBME280_measurement_ready();

    getBME280Values_async(temp, pressure, alt_barometric, hum);
}
