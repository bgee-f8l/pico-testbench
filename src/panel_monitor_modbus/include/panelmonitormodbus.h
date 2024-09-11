#ifndef PANELMONITORMODBUS_H
#define PANELMONITORMODBUS_H

#include "pico/stdlib.h"
#include "version.h"

#ifdef __cplusplus
extern "C" {
#endif

// code version
#define VERSION DEFAULT_VERSION

// MODBUS UART configuration
#define MODBUS_TX_PIN       4
#define MODBUS_RX_PIN       5

// Debug UART configuration
#define DEBUG_UART_ID       uart0
#define DEBUG_TX_PIN        0
#define DEBUG_RX_PIN        1
#define DEBUG_BAUD_RATE     921600

// Modbus RTU address
#define RTU_SERVER_ADDRESS 0x06

// define interlocks
#define GOOD    true
#define BAD     false

// Timer interval (ms) to service sensors
#define TIMER_FREQUENCY_HZ    20 
#define TIMER_PERIOD_MS (1000 / TIMER_FREQUENCY_HZ)

// define I2C pins
#define I2C_SDA_GPIO                 4
#define I2C_SCL_GPIO                 5

/**
 * Assign GPIO to server coil byte bits
 */

#define NOT_USED                     0

// coil gpio
#define INTERLOCK_OUT_GPIO           2
#define VOUT_ENABLE_OUT_GPIO         22

// coil address
#define INTERLOCK_OUT_ADDR          0x00
#define VOUT_ENABLE_OUT_ADDR        0x01

#define NUM_COILS                   2

/** 
 * Assign GPIO input byte bits
*/

// inputs
#define SYSTEM_INTERLOCK_IN_GPIO    3
#define SAFETY_INTERLOCK_IN_GPIO    6

// discrete input address
#define SYSTEM_INTERLOCK_IN_ADDR    0x00
#define SAFETY_INTERLOCK_IN_ADDR    0x01

#define NUM_INPUTS                  2

/** 
 * Assign holding registers input byte bits
*/

#define NUM_HOLDING_REGS            6
#define ADC_VREF                    3.0
#define ADC_BITS                    12
#define ADC_MAX                     ((1 << ADC_BITS) - 1)

// starting address is at 0x10 to account for reserved holding registers
#define PICO_ADC0_HOLDING_REG       0x10       // NOT USED
#define PICO_ADC1_HOLDING_REG       0x11       // NOT USED
#define PICO_ADC2_HOLDING_REG       0x12       // NOT USED

#define VOUT_SETPOINT_REG           0x13       // (uint16_t) 0 to 4095 corresponds to 0 to 100%
#define BITSPERVOLT_REG             0x14       // (uint16_t) VOUT bits per volt
#define DAC_VREF_REG                0x15       // (uint16_t) Measured Vref to DAC

#define LAST_HOLDING_REG            0x15        

uint16_t read_adc_channel(int adc_channel);

bool reserved_addr(uint8_t addr);

void init_server_i2c();

void init_server_adc();

void init_server_gpio();

void init_server_timers();

/** Read sensors and fill holding registers
 */
void read_server_sensors();

void set_vout_setpoint();

void get_vout_setpoint();

bool get_pico_interlock();

void set_pico_interlock(bool hilo);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif    //PANELMONITORMODBUS_H