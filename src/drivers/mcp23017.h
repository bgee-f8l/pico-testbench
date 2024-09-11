/*
 * Copyright (c) 2023 Fabric8Labs
 * Written by Michael Korody for Fabric8Labs
 * purpose is to controll Microchip MCP23017 io expander ic with raspberry pi pico
 * 
 */

#ifndef _MCP23017_H_
#define _MCP23017_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C bus address
#define MCP23017_I2C_ADDR   0x20

// MCP23017 Registers
#define IODIRA              0x00  // I/O Direction Register for Port A
#define IODIRB              0x01  // I/O Direction Register for Port B
#define GPIOA               0x12  // GPIO Port A
#define GPIOB               0x13  // GPIO Port B
#define IOCON               0x0A  //could be 0x15, 0x09, 0x0A

// macro to get pin position
#define GPIO_POSITION_TO_HEX(pos) (1 << (pos))

#define DIR_OUTPUT          0x00
#define DIR_INPUT           0xFF

/*! \brief Initialize the MCP23017 device
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 */
void init_mcp23017(i2c_inst_t *i2c_port);

/*! \brief Set the MCP23017 bank mode
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 */
void set_mcp23017_bank_mode(i2c_inst_t *i2c_port);

/*! \brief Set the bank mode
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 * \param port GPIOA (Port A) or GPIOB (port B).
 * \param direction Input or Output
 */
void set_mcp23017_gpio_direction(i2c_inst_t *i2c_port, uint8_t port, uint8_t direction);

/*! \brief Set single pin output value
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 * \param port GPIOA (Port A) or GPIOB (port B).
 * \param gpio The GPIO number
 * \param value 1 or 0, high or lo
 */
void set_mcp23017_gpio(i2c_inst_t *i2c_port, uint8_t port, uint8_t gpio, uint8_t value);

/*! \brief Get single pin output value
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 * \param port GPIOA (Port A) or GPIOB (port B).
 * \param gpio The GPIO number
 */
uint8_t get_mcp23017_gpio(i2c_inst_t *i2c_port, uint8_t port, uint8_t gpio);

/*! \brief Clear all outputs on a port.  Set the value to 0 or low
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 * \param port The MCP23017 port A or port B
 */
void clear_mcp23017_port(i2c_inst_t *i2c_port, uint8_t port);

/*! \brief Get all values on a port.
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1  
 * \param port GPIOA (Port A) or GPIOB (port B).
 */
uint8_t get_mcp23017_port(i2c_inst_t *i2c_port, uint8_t port);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  // _MCP23017_H
