/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "accel.h"

inline float convertRawGyro(int gRaw)
{
    return ((float)gRaw) * (((float)gyro_scale) / 32768.0);
}

inline float convertRawAccel(int aRaw)
{
    return ((float)aRaw) * (((float)accel_scale) / 32768.0);
}

//Accumulator buffer --- model dependent, but statically defined by the model
int64_t accumulatorArray[IMAI_DATA_OUT_COUNT];
int64_t accumulatorSum = 0;

int64_t accel_last_update = 0;
int64_t NN_last_get = 0;

void resetAccumulator()
{
    for (int i = 0; i < IMAI_DATA_OUT_COUNT; i++)
        accumulatorArray[i] = 0;
    accumulatorSum = 0;
}

void accumulate(int64_t pattern, int64_t timestamp)
{
    const int64_t delta = (timestamp - NN_last_get);
    accumulatorArray[pattern] += delta;
    accumulatorSum += delta;
}

long int getAccumulatorValue(const int pattern)
{
    return accumulatorArray[pattern];
}

long int getAccumulatorTime() { return accumulatorSum; }

void initAccel()
{
    // Initializes the I2C and the BMI16O
    // NB: this isn't fault tolerant (but the device is useless when the BMI160 is not available anway)
    BMI160.begin(BMI160GenClass::I2C_MODE, i2c_addr);

    BMI160.setGyroRange(gyro_scale);
    BMI160.setAccelerometerRange(accel_scale);

    BMI160.setGyroRate(gyro_rate);
    BMI160.setAccelerometerRate(accel_rate);

    IMAI_init();
    resetAccumulator();
}

void accel_read_state(const long int BMI160time)
{
    // Reads a the result the from the NN model
    IMAI_TIME_OUT_TYPE out_time[IMAI_TIME_OUT_COUNT];
    IMAI_DATA_OUT_TYPE state_prob_vector[IMAI_DATA_OUT_COUNT];

    IMAI_dequeue(state_prob_vector, out_time);

    //Find the most likely state
    int state = 0;
    float max_val = 0;
    for (int i = 0; i < IMAI_DATA_OUT_COUNT; i++)
        if (state_prob_vector[i] > max_val)
        {
            max_val = state_prob_vector[i];
            state = i;
        }

    //Pushes the result to the accumulator
    accumulate(state, BMI160time);

#ifdef DEBUGACCEL
    Serial.print(state_prob_vector[0]);
    Serial.print("\t");
    Serial.print(state_prob_vector[1]);
    Serial.print("\t");
    Serial.print(state_prob_vector[2]);
    Serial.print("\t");
    Serial.print(state_prob_vector[3]);
    Serial.print("\t");
    Serial.print("---");
    Serial.print("\t");
    Serial.print(state);
    Serial.println();
#endif
}

void update_accel(const long int BMI160time)
{ //Time of the measurement
    // Read raw gyro measurements from device
    int16_t rawgx, rawgy, rawgz, rawax, raway, rawaz; //buffer for raw values
    IMAI_DATA_IN_TYPE in_data_buffer[IMAI_DATA_IN_COUNT];

    BMI160.getMotion6(&rawax, &raway, &rawaz, &rawgx, &rawgy, &rawgz);

    // Convert to physical units
    float BMI160timeFloat = ((float)BMI160time) / 1e6;
    in_data_buffer[0] = convertRawAccel(rawax);
    in_data_buffer[1] = convertRawAccel(raway);
    in_data_buffer[2] = convertRawAccel(rawaz);
    in_data_buffer[3] = convertRawGyro(rawgx);
    in_data_buffer[4] = convertRawGyro(rawgy);
    in_data_buffer[5] = convertRawGyro(rawgz);  

    // Push the data to the NN model
    IMAI_enqueue(in_data_buffer, &BMI160timeFloat);
};

const long int accel_loop()
{
    // Accelerometer logic to be called repeatadly during the loop

    const int64_t BMI160time = esp_timer_get_time();
    const int64_t timeToUpdate = (MICROSECOND_PER_SECONDS / accel_rate) - (BMI160time - accel_last_update);

    // Time to read accelerometer data
    if ((timeToUpdate) <= 0)
    {
        update_accel(BMI160time);
        accel_last_update = BMI160time;
    }

    // Time to update state from NN
    if ((BMI160time - NN_last_get) >= 2 * MICROSECOND_PER_SECONDS)
    {
        accel_read_state(BMI160time);
        NN_last_get = BMI160time;
    }
    return timeToUpdate;
}
