/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _MCP4725_H
#define _MCP4725_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C bus address
#define MCP23017_I2C_ADDR   0x60

/**
 * @brief chip information definition
 */
#define CHIP_NAME                 "Microchip MCP4725"        /**< chip name */
#define MANUFACTURER_NAME         "Microchip"                /**< manufacturer name */
#define SUPPLY_VOLTAGE_MIN        2.7f                       /**< chip min supply voltage */
#define SUPPLY_VOLTAGE_MAX        5.5f                       /**< chip max supply voltage */
#define MAX_CURRENT               0.4f                       /**< chip max current */
#define TEMPERATURE_MIN           -40.0f                     /**< chip min operating temperature */
#define TEMPERATURE_MAX           125.0f                     /**< chip max operating temperature */
#define DRIVER_VERSION            2000                       /**< driver version */

/*! \brief Initialize the MCP4725 device
 * 
 * Put into power down mode and set ouput to zero volts.
 * 
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
void init_mcp4725(i2c_inst_t *i2c_port);

/*! \brief Power down the MCP4725
 *
 * Put into power down mode and set output to zero
 * 
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
void powerdown_mcp4725(i2c_inst_t *i2c_port);

/*! \brief Enable MCP4725.
 *
 * Disable power down mode and use to last setpoint.
 * 
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
void enable_mcp4725(i2c_inst_t *i2c_port);

/*! \brief Disable MCP4725
 *
 * Put into power down mode.  Does not change setpoint.
 * 
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
void disable_mcp4725(i2c_inst_t *i2c_port);

/*! \brief Set the output voltage setpoint
 *
 * Change the stored setpoint.
 * 
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
void set_mcp4725_setpoint(i2c_inst_t *i2c_port, uint16_t value);

/*! \brief Get the current output voltage setpoint
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
uint16_t get_mcp4725_setpoint(i2c_inst_t *i2c_port);

/*! \brief Get the MCP4725 setting
 *
 * \param i2c_port The I2C instance, either i2c0 or i2c1
 */
uint8_t get_mcp4725_setting(i2c_inst_t *i2c_port);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif    //_MCP4725_H

