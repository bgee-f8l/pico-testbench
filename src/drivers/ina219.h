
/**
 * Copyright (c) 2024 Fabric8Labs Inc.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _INA219_H
#define _INA219_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// I2C bus address
#define INA219_0_I2C_ADDR   0x20        //VGH
#define INA219_1_I2C_ADDR   0x20        //VSG
#define INA219_3_I2C_ADDR   0x20        //5V
#define INA219_4_I2C_ADDR   0x20        //3.3V

// Register addresses
#define INA219_REG_CONFIG      0x00
#define INA219_REG_SHUNTVOLTAGE 0x01
#define INA219_REG_BUSVOLTAGE   0x02
#define INA219_REG_POWER        0x03
#define INA219_REG_CURRENT      0x04
#define INA219_REG_CALIBRATION  0x05

// Calibration value
#define INA219_CALIBRATION_VALUE 4096

typedef struct {
    i2c_inst_t *i2c_port;
    uint8_t address;
    bool range;
    uint8_t pga;
    uint8_t badc;
    uint8_t sadc;
    uint8_t mode;
    uint16_t calibration;
} ina219_t;

/*! \brief Initialize the INA219 device
 *
 * \param device The I2C instance
 * \param device The I2C device address
 */
ina219_t* ina219_init(i2c_inst_t *i2c_port, uint8_t addr, ina219_t *device);

/*! \brief reads a register from the INA219 device
 *
 * \param device The INA219 instance
 * \param register The INA219 register address
 */
uint16_t inat219_read_register(ina219_t *device, uint8_t register);

/*! \brief reads a register from the INA219 device
 *
 * \param device The INA219 instance
 * \param register The INA219 register address
 * \param value the data to write to the register
 */
void ina219_write_register(ina219_t *device, uint8_t register, uint16_t value);

/*! \brief get the INA219 config
 *
 * \param device The INA219 instance
 */
ina219_t* ina219_read_config(ina219_t *device);

/*! \brief get the INA219 shunt voltage
 *
 * \param device The INA219 instance
 */
int16_t ina219_read_shunt_voltage(ina219_t *device);

/*! \brief get the INA219 bus voltage
 *
 * \param device The INA219 instance
 */
int16_t ina219_read_bus_voltage(ina219_t *device);

/*! \brief get the INA219 power
 *
 * \param device The INA219 instance
 */
int16_t ina219_read_power(ina219_t *device);

/*! \brief get the INA219 current
 *
 * \param device The INA219 instance
 */
int16_t ina219_read_current(ina219_t *device);

/*! \brief get the INA219 calibration
 *
 * \param device The INA219 instance
 */
ina219_t* ina219_read_calibration(ina219_t *device);

#define INA219_CONFIG_VALUE(range, pga, badc, sadc, mode)  \
    (((range & 0x01) << 13) |   /* Bus voltage range (BRNG) */    \
     ((pga & 0x03) << 9)   |    /* Gain selection (PGA) */        \
     ((badc & 0x0F) << 4)   |   /* Bus ADC resolution (BADC) */   \
     ((sadc & 0x0F) << 0)   |   /* Shunt ADC resolution (SADC) */ \
     ((mode & 0x07) << 0))      /* Operating mode (MODE) */

#define RANGE_16V       false
#define RANGE_32V       true
#define PGA_40MV        0b00
#define PGA_80MV        0b01
#define PGA_160MV       0b10
#define PGA_320MV       0b11

/*! \brief write the INA219 config
 *
 * \param device    The INA219 instance
 * \param range     bus voltage range
 * \param pga       shunt voltage pga
 * \param bads      bus adc resolution/averaging
 * \param sadc      shunt adc resolution/averaging
 */
void ina219_set_config(ina219_t *device);

/*! \brief get the INA219 shunt voltage
 *
 * \param device The INA219 instance
 */
void ina219_set_calibration(ina219_t *device);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif    //_INA219_H