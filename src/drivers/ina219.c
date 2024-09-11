
/**
 * Copyright (c) 2024 Fabric8Labs Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hardware/i2c.h"
#include "ina219.h"
#include "i2c_multicore/i2c_multicore.h"

// Helper function to write 16-bit register value
void ina219_write_register(ina219_t *device, uint8_t reg, uint16_t value) {
    uint8_t buffer[3];
    buffer[0] = reg;
    buffer[1] = (value >> 8) & 0xFF;
    buffer[2] = value & 0xFF;
    i2c_write_multicore(device->i2c_port, device->address, buffer, 3, false);
}

// Helper function to read 16-bit register value
int16_t ina219_read_register(ina219_t *device, uint8_t reg) {
    uint8_t buffer[2];
    i2c_write_multicore(device->i2c_port, device->address, &reg, 1, true);
    i2c_read_multicore(device->i2c_port, device->address, buffer, 2, false);
    return (buffer[0] << 8) | buffer[1];
}

// Initialization function
ina219_t* ina219_init(i2c_inst_t *i2c_port, uint8_t addr, ina219_t *device) {
    // Default calibration and configuration    
    device->i2c_port = i2c_port;
    device->address = addr;
    device->range = RANGE_16V;
    device->pga = PGA_320MV;
    device->badc = 0b1001;
    device->sadc = 0b1001;
    device->mode = 0b111;
    device->calibration = 0;

    //set config
    ina219_set_config(device);

    return device;    
}

// Calibration function for the INA219
void ina219_set_calibration(ina219_t *device) {
    ina219_write_register(device, INA219_REG_CALIBRATION, device->calibration);
}

ina219_t* ina219_get_calibration(ina219_t *device) {
    // not implemented

    return device;
}

// Config function for the INA219
void ina219_set_config(ina219_t *device) {
    uint16_t config = INA219_CONFIG_VALUE(device->range, device->pga, 
                                            device->badc, device->sadc, 
                                            device->mode);
    ina219_write_register(device, INA219_REG_CONFIG, config);
}

ina219_t* ina219_read_config(ina219_t *device) {
    // not implemented
    return device;
}

// Function to read shunt voltage (in microvolts)
int16_t ina219_read_shunt_voltage(ina219_t *device) {
    int16_t shunt_voltage = ina219_read_register(device, INA219_REG_SHUNTVOLTAGE);
    return shunt_voltage;  // Value is in microvolts
}

// Function to read bus voltage (in millivolts)
int16_t ina219_read_bus_voltage(ina219_t *device) {
    int16_t bus_voltage = ina219_read_register(device, INA219_REG_BUSVOLTAGE);
    bus_voltage = (bus_voltage >> 3) * 4;  // Scale to millivolts
    return bus_voltage;
}

// Function to read current (in milliamps)
int16_t ina219_read_current(ina219_t *device) {
    int16_t current = ina219_read_register(device, INA219_REG_CURRENT);
    return current;  // Value is in milliamps
}

// Function to read power (in milliamps)
int16_t ina219_read_power(ina219_t *device) {
    int16_t power = ina219_read_register(device, INA219_REG_POWER);
    return power;  // Value is in milliamps
}