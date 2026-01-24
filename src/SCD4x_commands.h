#ifndef SCD4X_COMMANDS_H
#define SCD4X_COMMANDS_H

#include <Arduino.h>

/*
These commands are the commands for the SCD4X sensor family
For further reference look into the Datasheet of the sensor
SCD41 datasheet: https://download.mikroe.com/documents/datasheets/SCD41%20Datasheet.pdf

Usage for every 16 bit command
Wire.write(command >> 8);   
Wire.write(command & 0xFF);
*/ 
namespace SCD4X {
    // I2C Address
    constexpr uint8_t I2C_ADDR = 0x62;

    // Basic Commands (Chapter 3.5)
    constexpr uint16_t START_PERIODIC_MEASUREMENT = 0x21B1;
    constexpr uint16_t READ_MEASUREMENT           = 0xEC05;
    constexpr uint16_t STOP_PERIODIC_MEASUREMENT  = 0x3F86;

    // On-chip output signal compensation (Chapter 3.6)
    constexpr uint16_t SET_TEMPERATURE_OFFSET     = 0x241D;
    constexpr uint16_t GET_TEMPERATURE_OFFSET     = 0x2318;
    constexpr uint16_t SET_SENSOR_ALTITUDE        = 0x2427;
    constexpr uint16_t GET_SENSOR_ALTITUDE        = 0x2322;
    constexpr uint16_t SET_AMBIENT_PRESSURE       = 0xE000;

    // Field calibration (Chapter 3.7)
    constexpr uint16_t PERFORM_FORCED_RECALIBRATION   = 0x362F;
    constexpr uint16_t SET_AUTO_SELF_CALIBRATION_EXE  = 0x2416; // ASC Enabled
    constexpr uint16_t GET_AUTO_SELF_CALIBRATION_EXE  = 0x2313;

    // Low power (Chapter 3.8)
    constexpr uint16_t START_LOW_POWER_MEASUREMENT    = 0x21AC;
    constexpr uint16_t GET_DATA_READY_STATUS          = 0xE4B8;

    // Advanced features (Chapter 3.9)
    constexpr uint16_t PERSIST_SETTINGS   = 0x3615;
    constexpr uint16_t GET_SERIAL_NUMBER  = 0x3682;
    constexpr uint16_t PERFORM_SELF_TEST  = 0x3639;
    constexpr uint16_t PERFORM_FACTORY_RESET = 0x3632;
    constexpr uint16_t REINIT             = 0x3646;

    // Low power single shot (Chapter 3.10 - SCD41 only)
    constexpr uint16_t MEASURE_SINGLE_SHOT          = 0x219D;
    constexpr uint16_t MEASURE_SINGLE_SHOT_RHT_ONLY = 0x2196;

    // Calculation of Temperature
    inline float calculateTemp(uint16_t raw) {
        return -45.0f + 175.0f * (float)raw / 65536.0f;
    }

    // Calculation of Humidity
    inline float calculateHum(uint16_t raw) {
        return 100.0f * (float)raw / 65536.0f;
    }
}

#endif