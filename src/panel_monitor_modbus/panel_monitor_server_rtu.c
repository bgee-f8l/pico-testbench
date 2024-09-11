/*
   This example application sets up an RTU server and handles modbus requests

   This server supports the following function codes:
   FC 01 (0x01) Read Coils   
   FC 02 (0x02) Read Input
   FC 03 (0x03) Read Holding Registers
   FC 15 (0x0F) Write Multiple Coils
   FC 16 (0x10) Write Multiple Registers
*/

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "heartbeat/heartbeat.h"
#include "usb_printf/usb_printf.h"
#include "i2c_multicore/i2c_multicore.h"
#include "panelmonitormodbus.h"
#include "ina219.h"
#include "nanomodbus.h"
#include "rs485.h"
#include "version.h"
#include "pico/multicore.h" // multicore_reset_core1

extern const uint8_t server_input_gpio[];
extern const uint8_t server_coil_gpio[];
extern volatile uint16_t read_sensor_flag;
extern volatile uint16_t blink_timer_flag;
extern volatile uint16_t pps_timer_flag;

//panel monitors
ina219_t vgh_monitor;
ina219_t vsh_monitor;
ina219_t v5_monitor;
ina219_t v3p3_monitor;

typedef struct {
  uint64_t time_ms;
  int16_t vgh_bus_v;
  int16_t vgh_shunt_v;
  int16_t vsh_bus_v;
  int16_t vsh_shunt_v;
  int16_t v5_bus_v;
  int16_t v5_shunt_v;
  int16_t v3p3_bus_v;
  int16_t v3p3_shunt_v;
} panel_power_t;

// The data model of this sever will support coils addresses 0 to 100 and registers addresses from 0 to 32
#define COILS_ADDR_MAX NUM_COILS
#define INPUTS_ADDR_MAX NUM_INPUTS

// define last address.
#define LAST_REG_ADDR LAST_HOLDING_REG

// Debug flag
#define DEBUG false

// A single nmbs_bitfield variable can keep 2000 coils
nmbs_bitfield server_coils = {0};
uint16_t server_registers[LAST_REG_ADDR + 1] = {0};

int32_t read_serial(uint8_t *buf, uint16_t count, int32_t byte_timeout_ms, void *arg)
{
    /*
    This methods should block until either:
    - count bytes of data are read/written
    - the byte timeout, with byte_timeout_ms >= 0, expires

    A value < 0 for byte_timeout_ms means no timeout.

    Their return value should be the number of bytes actually read/written, or < 0 in case of error.
    A return value between 0 and count - 1 will be treated as if a timeout occurred on the transport side. All other values will be treated as transport errors.
    */
    if (DEBUG) printf("Reading %d bytes with timeout %d\n", count, byte_timeout_ms);

    absolute_time_t start_time = get_absolute_time();

    for (int i = 0; i < count; i++) 
    {
        while (!uart_is_readable(RS485_UART_ID)) 
        {
            if ( (absolute_time_diff_us(start_time, get_absolute_time()) / 1000) > byte_timeout_ms ) 
            {
                if (DEBUG) printf("Timeout after %d ms of reading %d bytes\n", byte_timeout_ms, i);
                return i;
            }
        }

        buf[i] = uart_getc(RS485_UART_ID);
        printf("Read byte: %d (0x%02X)\n", buf[i], buf[i]);
    }

    printf("====================================\n");

    return count;
}

int32_t write_serial(const uint8_t *buf, uint16_t count, int32_t byte_timeout_ms, void *arg)
{
    /*
    This methods should block until either:
    - count bytes of data are read/written
    - the byte timeout, with byte_timeout_ms >= 0, expires

    A value < 0 for byte_timeout_ms means no timeout.

    Their return value should be the number of bytes actually read/written, or < 0 in case of error.
    A return value between 0 and count - 1 will be treated as if a timeout occurred on the transport side. All other values will be treated as transport errors.
    */

    if (DEBUG) printf("Writing %d bytes with timeout %d\n", count, byte_timeout_ms);

    absolute_time_t start_time = get_absolute_time();

    for (int i = 0; i < count; i++) 
    {
        while (!uart_is_writable(RS485_UART_ID)) 
        {
            if ( byte_timeout_ms >= 0 ) 
            {
                if ( (absolute_time_diff_us(start_time, get_absolute_time()) / 1000) > byte_timeout_ms ) 
                {
                    if (DEBUG) printf("Timeout after %d ms of writing %d bytes\n", byte_timeout_ms, i);
                    return i;
                }
            }
        }

        printf("Writing byte: %d (0x%02X)\n", buf[i], buf[i]);
        uart_putc(RS485_UART_ID, buf[i]);
    }

    printf("====================================\n");

    return count;
}

void onError() {
  // Set the led ON on error
  while (true) {
    gpio_put(LED_PIN, ON);
    // digitalWrite(LED_BUILTIN, HIGH);
  }
}

/* Function 0x01 Read Coil Status 
** Example: https://www.simplymodbus.ca/FC01.htm
*/
nmbs_error handle_read_coils(uint16_t address, uint16_t quantity, nmbs_bitfield coils_out, void *arg) {
  printf("Read coils called with address: %d | quantity: %d\n", address, quantity);

  if (address + quantity > COILS_ADDR_MAX + 1)
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

  // Read our coils values into coils_out
  for (int i = 0; i < quantity; i++) {
    bool value = nmbs_bitfield_read(server_coils, address + i);
    nmbs_bitfield_write(coils_out, i, value);
  }

  return NMBS_ERROR_NONE;
}

/* Function 0x02 Read Input Status 
** Example: https://www.simplymodbus.ca/FC02.htm
*/
nmbs_error handle_read_discrete(uint16_t address, uint16_t quantity, nmbs_bitfield inputs_out, void *arg) {
  printf("Read input discrete called with address: %d | quantity: %d\n", address, quantity);

  if (address + quantity > INPUTS_ADDR_MAX + 1)
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

  for (int i = 0; i < quantity; i++) {
    int index = address + i;
    int gpio_pin = server_input_gpio[index];
    bool value = gpio_get(gpio_pin);
    nmbs_bitfield_write(inputs_out, i, value);
  }

  return NMBS_ERROR_NONE; 
}

/* Function 0x03 Read holding registers 
** Example: https://www.simplymodbus.ca/FC03.htm
*/
nmbs_error handler_read_holding_registers(uint16_t address, uint16_t quantity, uint16_t* registers_out, void *arg) {
  printf("Read holding registers called with address: %d | quantity: %d\n", address, quantity);
  
  if (address + quantity > LAST_REG_ADDR + 1)
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

  // Read our registers values into registers_out
  for (int i = 0; i < quantity; i++)
    registers_out[i] = server_registers[address + i];

  return NMBS_ERROR_NONE;
}

/* Function 0x0F Write multiple coils 
** Example: https://www.simplymodbus.ca/FC15.htm
*/
nmbs_error handle_write_multiple_coils(uint16_t address, uint16_t quantity, const nmbs_bitfield coils, void *arg) {
  printf("Write multiple coils called with address: %d | quantity: %d\n", address, quantity);

  if (address + quantity > COILS_ADDR_MAX + 1)
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

  // Write coils values to our server_coils
  for (int i = 0; i < quantity; i++) {
    uint16_t coil_address = address + i;
    nmbs_bitfield_write(server_coils, address + i, nmbs_bitfield_read(coils, i));      
    printf("coil address: %x", coil_address);
    // set coil value   
    switch (coil_address) {
      default: 
        gpio_put(server_coil_gpio[address + i], nmbs_bitfield_read(coils, i));
        break;
    }
  }
  
  return NMBS_ERROR_NONE;
}

/* Function 0x10 Write multiple registers 
** Example: https://www.simplymodbus.ca/FC16.htm
*/
nmbs_error handle_write_multiple_registers(uint16_t address, uint16_t quantity, const uint16_t* registers, void *arg) {
  printf("Write multiple registers called with address: %d | quantity: %d\n", address, quantity);

  if (address + quantity > LAST_REG_ADDR + 1)
    return NMBS_EXCEPTION_ILLEGAL_DATA_ADDRESS;

  // Write registers values to our server_registers
  for (int i = 0; i < quantity; i++) {
    server_registers[address + i] = registers[i];
  }

  return NMBS_ERROR_NONE;
}

void setup()
{
    // Initialize the builtin LED (used for error indication)
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);

    // Initialize the usb output
    stdio_usb_init();
    stdio_set_driver_enabled(&stdio_usb, false);  // disable general output (printf) on usb.

    // Initialize the debug UART
    stdio_uart_init_full(DEBUG_UART_ID, DEBUG_BAUD_RATE, DEBUG_TX_PIN, DEBUG_RX_PIN);
    uart_set_hw_flow(DEBUG_UART_ID, false, false);
    uart_set_format(DEBUG_UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Initialize the RS485 UART
    uart_init(RS485_UART_ID, BAUD_RATE);

    gpio_set_function(RS485_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS485_UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(RS485_UART_ID, false, false);
    uart_set_format(RS485_UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Initialize the RS485 RTS pin
    gpio_init(RS485_UART_RTS_PIN);
    gpio_set_dir(RS485_UART_RTS_PIN, GPIO_OUT);
    gpio_put(RS485_UART_RTS_PIN, OFF); // Set RTS to low initially (receive enabled)

    // init i2C channel
    init_i2c_multicore(i2c0, I2C_SDA_GPIO, I2C_SCL_GPIO);

    // init ina219
    ina219_init(i2c0, 0x45, &vgh_monitor);  //0x40
    ina219_init(i2c0, 0x45, &vsh_monitor);  //0x41
    ina219_init(i2c0, 0x45, &v5_monitor);   //0x44
    v3p3_monitor.pga = PGA_80MV;
    ina219_init(i2c0, 0x45, &v3p3_monitor); //0x45

    // init gpio
    init_server_gpio();

    // init adc
    init_server_adc();

    // init timers
    init_server_timers();

    // init heartbeat
    init_heartbeat(RTU_SERVER_ADDRESS);

    // init version number
    server_registers[VERSION_NUMBER_REG] = VERSION;

    // init commit_short_hash
    uint32_t short_hash = COMMIT_SHORT_HASH;

    server_registers[COMMIT_SHORT_HASH0_REG] = (uint16_t)(short_hash & 0xFFFF);
    server_registers[COMMIT_SHORT_HASH1_REG] = (uint16_t)((short_hash >> 16) & 0xFFFF);    
}

void core1_entry() {
  while(1) {
  }
}

int main() {

  multicore_launch_core1(core1_entry); // Start core 1 

  setup();

  nmbs_platform_conf platform_conf = {0};
  platform_conf.transport = NMBS_TRANSPORT_RTU;
  platform_conf.read = read_serial;
  platform_conf.write = write_serial;
  platform_conf.arg = NULL;

  nmbs_callbacks callbacks = {0};
  callbacks.read_coils = handle_read_coils;
  callbacks.read_discrete_inputs = handle_read_discrete;
  callbacks.write_multiple_coils = handle_write_multiple_coils;
  callbacks.read_holding_registers = handler_read_holding_registers;
  callbacks.write_multiple_registers = handle_write_multiple_registers;
  
  // Create the modbus server
  nmbs_t nmbs;
  nmbs_error err = nmbs_server_create(&nmbs, RTU_SERVER_ADDRESS, &platform_conf, &callbacks);
  if (err != NMBS_ERROR_NONE) {
    onError();
  }

  nmbs_set_read_timeout(&nmbs, 5);
  nmbs_set_byte_timeout(&nmbs, 1);

  // initialize pico interlock
  bool pico_interlock = GOOD;
  set_pico_interlock(GOOD);

  printf("Starting Power Supply Modbus server ...\n\r");
  printf("Version: %d, Short Hash: 0x%X\r\n", VERSION, COMMIT_SHORT_HASH);
  
  while (true) {
    err = nmbs_server_poll(&nmbs);
    // This will probably never happen, since we don't return < 0 in our platform funcs
    if (err == NMBS_ERROR_TRANSPORT)
      break;

    if (read_sensor_flag == 1) {
        // do something
        read_server_sensors();
        //pico_interlock = check_sensor_thresholds();
        panel_power_t readings;
        uint64_t timestamp_us = to_us_since_boot(get_absolute_time());
        readings.vgh_bus_v = ina219_read_bus_voltage(&vgh_monitor);
        readings.vgh_shunt_v = ina219_read_shunt_voltage(&vgh_monitor);
        readings.vsh_bus_v = ina219_read_bus_voltage(&vsh_monitor);
        readings.vsh_shunt_v = ina219_read_shunt_voltage(&vsh_monitor);
        readings.v5_bus_v = ina219_read_bus_voltage(&v5_monitor);
        readings.v5_shunt_v = ina219_read_shunt_voltage(&v5_monitor);
        readings.v3p3_bus_v = ina219_read_bus_voltage(&v3p3_monitor);
        readings.v3p3_shunt_v = ina219_read_shunt_voltage(&v3p3_monitor);
        usb_printf("0,%" PRId64 ",%d,%d,%d,%d,%d,%d,%d,%d\r\n", timestamp_us,
                                                                readings.vgh_bus_v,
                                                                readings.vgh_shunt_v,
                                                                readings.vsh_bus_v,
                                                                readings.vsh_shunt_v,
                                                                readings.v5_bus_v,
                                                                readings.v5_shunt_v,
                                                                readings.v3p3_bus_v,
                                                                readings.v3p3_shunt_v);

        read_sensor_flag = 0;
    }

    if (pico_interlock == BAD || !gpio_get(SAFETY_INTERLOCK_IN_GPIO) || !gpio_get(SYSTEM_INTERLOCK_IN_GPIO)) {
      set_pico_interlock(BAD);
    } else {
      set_pico_interlock(GOOD);
    }
  }

  // No need to destroy the nmbs instance, terminate the program
  return 0;
}
