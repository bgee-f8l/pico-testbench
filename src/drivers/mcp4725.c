
/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <math.h>
#include "hardware/i2c.h"
#include "mcp4725.h"
#include "i2c_multicore/i2c_multicore.h"

void init_mcp4725(i2c_inst_t *i2c_port) {
  powerdown_mcp4725(i2c_port);
}

void powerdown_mcp4725(i2c_inst_t *i2c_port) {
  uint8_t data[3];

  data[0] = 0b0001 << 4; // set to power down mode with 1k
  data[0] |= (0x00 >> 8) & 0x0F; // Upper 4 bits of value
  data[1] = 0x00 & 0xFF; // Lower 8 bits of value

  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 3, false);
}

void disable_mcp4725(i2c_inst_t *i2c_port) {
  uint16_t setpoint = get_mcp4725_setpoint(i2c_port);

  uint8_t data[3];

  data[0] = 0b0001 << 4; // set to power down mode with 1k
  data[0] |= (setpoint >> 8) & 0x0F; // Upper 4 bits of value
  data[1] = setpoint & 0xFF; // Lower 8 bits of value

  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 3, false);
}

void enable_mcp4725(i2c_inst_t *i2c_port) {
  uint16_t setpoint = get_mcp4725_setpoint(i2c_port);

  uint8_t data[3];

  data[0] = 0b0000 << 4; // set to enable
  data[0] |= (setpoint >> 8) & 0x0F; // Upper 4 bits of value
  data[1] = setpoint & 0xFF; // Lower 8 bits of value

  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 3, false);
}

void set_mcp4725_setpoint(i2c_inst_t *i2c_port, uint16_t value) {
  uint8_t data[3];
  
  uint8_t setting = (get_mcp4725_setting(i2c_port) >> 1) & 0b011;

  data[0] = setting << 4; // Write DAC Register
  data[0] |= (value >> 8) & 0x0F; // Upper 4 bits of value
  data[1] = value & 0xFF; // Lower 8 bits of value

  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 3, false);
}

uint16_t get_mcp4725_setpoint(i2c_inst_t *i2c_port) {
  uint16_t value = 0;

  // read dac setting
  uint8_t rxdata[6];

  i2c_read_multicore(i2c_port, MCP23017_I2C_ADDR, rxdata, 5, false);
  value = rxdata[1] << 4;
  value |= (rxdata[2] >> 4) & 0x0F;  
  return value;
}

uint8_t get_mcp4725_setting(i2c_inst_t *i2c_port) {  
  uint8_t rxdata;

  i2c_read_multicore(i2c_port, MCP23017_I2C_ADDR, &rxdata, 1, false);
  return rxdata;
}

