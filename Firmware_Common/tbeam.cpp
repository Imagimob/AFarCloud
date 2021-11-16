/* 
MIT License
Copyright 2021 AFarCloud consortium, Imagimob AB, Nicolas Innocenti, Alex Jonsson
*/

#include "tbeam.h"
#include <Wire.h>
#include <axp20x.h> // https://github.com/Imagimob/AXP202X_Library

// LoRa pins mapping
extern const lmic_pinmap lmic_pins = {
    .nss = 18,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 23,
    .dio = {26, 33, 32},
};

#ifdef DEBUG
void printBatteryVoltage()
{
    Serial.print("Battery voltage: ");
    Serial.print(getBatteryVoltage());
    Serial.println("V");
};
#endif //DEBUG

    /** 
 *   Note: TTGO comes in 2 versions with different pin mapping for the GPS.
 *   The new board has an axp192 power supply which can be detected by the software at runtime.
 *   
 * 
 *    Old boards - 20180711 is old
 *    GPS_TX 12
 *    GPS_RX 15
 *    BUILTIN_LED 14 // T-Beam blue LED, see: http://tinymicros.com/wiki/TTGO_T-Beam
 * 
 *    New boards - 20190612 is new
 *    GPS_TX 34
 *    GPS_RX 12
 *    BUILTIN_LED 14 // T-Beam blue LED (?), see: http://tinymicros.com/wiki/TTGO_T-Beam
 * 
 * 
 **/

    /**
 * Init the power manager chip
 * 
 * axp192 power 
    DCDC1 0.7-3.5V @ 1200mA max -> OLED // If you turn this off you'll lose comms to the axp192 because the OLED and the axp192 share the same i2c bus, instead use ssd1306 sleep mode
    DCDC2 -> unused
    DCDC3 0.7-3.5V @ 700mA max -> ESP32 (keep this on!)
    LDO1 30mA -> charges GPS backup battery // charges the tiny J13 battery by the GPS to power the GPS ram (for a couple of days), can not be turned off
    LDO2 200mA -> LORA
    LDO3 200mA -> GPS
 */

#define BATTERY_PIN 35 // battery level measurement pin (old board)

AXP20X_Class axp;

bool axp192_found = false;  // Tells if axp192 was found, essentially telling us if we are running on old or new board
bool ssd1306_found = false; // Tells us if the OLED screen was found (not used here)
bool pmu_irq = false;
#define PMU_IRQ 35
String baChStatus = "No charging";

void axp192Init(bool GPSON)
{
    if (axp192_found)
    {
#ifdef DEBUG
        if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS))
        {
            Serial.println("AXP192 Begin PASS");
        }
        else
        {
            Serial.println("AXP192 Begin FAIL");
        }
        // axp.setChgLEDMode(LED_BLINK_4HZ);
        Serial.printf("DCDC1: %s\n", axp.isDCDC1Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("DCDC2: %s\n", axp.isDCDC2Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("LDO2: %s\n", axp.isLDO2Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("LDO3: %s\n", axp.isLDO3Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("DCDC3: %s\n", axp.isDCDC3Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("Exten: %s\n", axp.isExtenEnable() ? "ENABLE" : "DISABLE");
        Serial.println("----------------------------------------");
#endif

        axp.setPowerOutPut(AXP192_LDO2, AXP202_ON); // LORA radio

        if (GPSON)
            axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
        else
            axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // GPS main power

        axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
        axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
        axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
        axp.setDCDC1Voltage(3300); // for the OLED power

#ifdef DEBUG
        Serial.printf("DCDC1: %s\n", axp.isDCDC1Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("DCDC2: %s\n", axp.isDCDC2Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("LDO2: %s\n", axp.isLDO2Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("LDO3: %s\n", axp.isLDO3Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("DCDC3: %s\n", axp.isDCDC3Enable() ? "ENABLE" : "DISABLE");
        Serial.printf("Exten: %s\n", axp.isExtenEnable() ? "ENABLE" : "DISABLE");
#endif
        pinMode(PMU_IRQ, INPUT_PULLUP);
        attachInterrupt(
            PMU_IRQ, []
            { pmu_irq = true; },
            FALLING);

        axp.adc1Enable(AXP202_BATT_CUR_ADC1, 1);
        axp.enableIRQ(AXP202_VBUS_REMOVED_IRQ | AXP202_VBUS_CONNECT_IRQ | AXP202_BATT_REMOVED_IRQ | AXP202_BATT_CONNECT_IRQ, 1);
        axp.clearIRQ();

        if (axp.isChargeing())
        {
            baChStatus = "Charging";
        }
    }
    else
    {
#ifdef DEBUG
        Serial.println("AXP192 not found");
#endif
    }
}

float getBatteryVoltage()
{
    if (axp192_found)
    {
#ifdef DEBUG
        Serial.println("Battery: AXP192 found");
#endif
        float battery_voltage = axp.getBattVoltage() / 1000.0;
#ifdef DEBUG
        Serial.print("Voltage:");
        Serial.println(battery_voltage);
#endif
        return battery_voltage;
    }
    else
    {
        // we've set 10-bit ADC resolution 2^10=1024 and voltage divider makes it half of maximum readable value (which is 3.3V)
        return analogRead(BATTERY_PIN) * 2.0 * (3.3 / 1024.0);
    }
}

#define SSD1306_ADDRESS 0x3C

void scanI2Cdevice(void)
{
    byte err, addr;
    int nDevices = 0;
    for (addr = 1; addr < 127; addr++)
    {
        Wire.beginTransmission(addr);
        err = Wire.endTransmission();
        if (err == 0)
        {
#ifdef DEBUG
            Serial.print("I2C device found at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.print(addr, HEX);
            Serial.println(" !");
#endif
            nDevices++;

            if (addr == SSD1306_ADDRESS)
            {
                ssd1306_found = true;
#ifdef DEBUG
                Serial.println("ssd1306 display found");
#endif
            }
            if (addr == AXP192_SLAVE_ADDRESS)
            {
                axp192_found = true;
#ifdef DEBUG
                Serial.println("axp192 PMU found");
#endif
            }
        }
        else if (err == 4)
        {
#ifdef DEBUG
            Serial.print("Unknow error at address 0x");
            if (addr < 16)
                Serial.print("0");
            Serial.println(addr, HEX);
#endif
        }
    }
#ifdef DEBUG
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
#endif
}

#define I2C_SDA 21
#define I2C_SCL 22
void setup_TTGO(bool gpson)
{
    Wire.begin(I2C_SDA, I2C_SCL);
    scanI2Cdevice();
    axp192Init(gpson);

    // set battery measurement pin
    if (!axp192_found)
    {
        adcAttachPin(BATTERY_PIN);
        analogReadResolution(10); // Default of 12 is not very linear. Recommended to use 10 or 11 depending on needed resolution.
    }

    //Turn off WiFi and Bluetooth
    WiFi.mode(WIFI_OFF);
    btStop();
};

// Dynamically set GPS pins
int8_t getGPSRXpin()
{
    if (axp192_found)
        return ((int8_t)12);
    return ((int8_t)15);
};

int8_t getGPSTXpin()
{
    if (axp192_found)
        return ((int8_t)34);
    return ((int8_t)12);
};
