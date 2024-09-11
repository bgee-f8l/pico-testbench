
/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <math.h>
#include "hardware/i2c.h"
#include "mcp23017.h"
#include "i2c_multicore/i2c_multicore.h"

void init_mcp23017(i2c_inst_t *i2c_port) {
  set_mcp23017_bank_mode(i2c_port);
  set_mcp23017_gpio_direction(i2c_port, IODIRA, DIR_OUTPUT);
  set_mcp23017_gpio_direction(i2c_port, IODIRB, DIR_OUTPUT);
  clear_mcp23017_port(i2c_port, GPIOA);
  clear_mcp23017_port(i2c_port, GPIOB);
}

void set_mcp23017_bank_mode(i2c_inst_t *i2c_port) {
  uint8_t data[2];
  data[0] = IOCON;
  data[1] = 0x20; 
  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 2, false);
}

void set_mcp23017_gpio_direction(i2c_inst_t *i2c_port, uint8_t port, uint8_t direction) 
{
  uint8_t data[2];
  data[0] = port;
  data[1] = direction;
  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 2, false);
}

void set_mcp23017_gpio(i2c_inst_t *i2c_port, uint8_t port, uint8_t gpio, uint8_t value)
{        
  printf("setting Port %s:%d %s\n\r", port == GPIOA ? "A" : "B", value == true ? "high" : "low");
  
  // get current port values
  uint8_t port_values = get_mcp23017_port(i2c_port, port);

  uint8_t data[2];
  data[0] = port;
  if (value) {
    data[1] = port_values | (GPIO_POSITION_TO_HEX(gpio)); // set gpio high
  } else {
    data[1] = port_values & ~(GPIO_POSITION_TO_HEX(gpio)); // set gpio low
  }  
  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 2, false);
}

void clear_mcp23017_port(i2c_inst_t *i2c_port, uint8_t port) 
{  
  uint8_t data[2];
  data[0] = port;
  data[1] = 0x00;  // clear that gpio value
  i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, data, 2, false);
}

uint8_t get_mcp23017_gpio(i2c_inst_t *i2c_port, uint8_t port, uint8_t gpio) 
{
    // Read the value of a specific gpio    
    uint8_t rxdata[1];
    
    // Read GPIO register
    i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, &port, 1, true);
    i2c_read_multicore(i2c_port, MCP23017_I2C_ADDR, rxdata, 1, false);
    
    // Extract the value of the specified pin
    return (rxdata[0] >> gpio) & 1;
}

uint8_t get_mcp23017_port(i2c_inst_t *i2c_port, uint8_t port) 
{
    // Read the value of a specific gpio
    uint8_t reg_addr = GPIOA;
    uint8_t rxdata[2];
    
    // Read GPIO register
    i2c_write_multicore(i2c_port, MCP23017_I2C_ADDR, &reg_addr, 1, true);
    i2c_read_multicore(i2c_port, MCP23017_I2C_ADDR, rxdata, 2, false);

    // return the whole port   
    if (port == GPIOA) 
      return rxdata[0];
    else
      return rxdata[1];
}



