/*
 * For G30-EB0120-R0
 * writen by Michael Korody and Brandon Gee sept 2023 for Fabric8Labs
 * 10 MHz linear decoder powered by rasberry pi pico
 * additional 3x analog inputs, gpio bank and 485 comms
 * 
*/

#include <stdio.h>
#include "pico/time.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "panelmonitormodbus.h"
#include "nanomodbus.h"

// DEBUG flag
#define SENSOR_DEBUG false

extern nmbs_bitfield server_coils;
extern uint16_t server_registers[];

// these are used by the timer interrupt routine
const uint16_t timer_period = 1000 / TIMER_FREQUENCY_HZ;
volatile uint16_t read_sensor_flag;
volatile uint16_t blink_timer_flag;
volatile uint16_t pps_timer_flag;
volatile absolute_time_t read_sensor_time;
volatile int32_t pulse_count_snapshot = 0;
volatile uint16_t timer_hits = 0;

// for frequency counter
uint32_t seconds = 0;
uint32_t prev_count = 0;

// input addresses                                                        
const uint8_t server_input_gpio[NUM_INPUTS] = { SYSTEM_INTERLOCK_IN_GPIO,
                                            SAFETY_INTERLOCK_IN_GPIO };

// coil addresses                                                         
const uint8_t server_coil_gpio[NUM_COILS] = { INTERLOCK_OUT_GPIO,
                                          VOUT_ENABLE_OUT_GPIO };

uint16_t pps_count = 0;

// setup blink and pps timer callback
bool blink_timer_callback() {
  blink_timer_flag = 1;
  pps_count++;
  if (pps_count == 5)
  {
    pps_count = 0;
    pps_timer_flag = 1;
  }
  return true;
}

// setup timer callback
bool read_sensor_timer_callback() {
    read_sensor_time = get_absolute_time();
    read_sensor_flag = 1;
    timer_hits++;

    return true;
}

void set_pico_interlock(bool value) {
  // set coil interlock
  gpio_put(INTERLOCK_OUT_GPIO, value);

  // update server coils register
  nmbs_bitfield_write(server_coils, INTERLOCK_OUT_ADDR, value);
}

bool get_pico_interlock() {
  return nmbs_bitfield_read(server_coils, INTERLOCK_OUT_ADDR);
}

// setup ultra controller interlock callback
void gpio_callback(uint gpio, uint32_t events) {
  switch (gpio)
  {
    case SYSTEM_INTERLOCK_IN_GPIO:
      if (events == GPIO_IRQ_EDGE_FALL) {
          set_pico_interlock(BAD);
#if SENSOR_DEBUG
    printf("SYSTEM_INTERLOCK tripped\n");
#endif
      }

      if (events == GPIO_IRQ_EDGE_RISE) {
#if SENSOR_DEBUG
    printf("SYSTEM_INTERLOCK set\n");
#endif
      }
    break;

    case SAFETY_INTERLOCK_IN_GPIO:
      if (events == GPIO_IRQ_EDGE_FALL) {
          set_pico_interlock(BAD);
#if SENSOR_DEBUG
    printf("SAFETY_INTERLOCK tripped\n");
#endif
      }

      if (events == GPIO_IRQ_EDGE_RISE) {
#if SENSOR_DEBUG
    printf("SAFETY_INTERLOCK set\n");
#endif
      }
    break;
  }
}

uint16_t read_adc_channel(int adc_channel) {
    // 12-bit conversion, assume max value == ADC_VREF
    adc_select_input(adc_channel);
    uint16_t result = adc_read();

#if SENSOR_DEBUG
  if (timer_hits % TIMER_FREQUENCY_HZ == 0) {
    const float conversion_factor = ADC_VREF / (1 << 12);
    printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor); 
  }
#endif

    return result;
}

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void init_server_gpio() { 
  for (int i = 0; i < NUM_COILS; i++) {    
    int gpio_pin = server_coil_gpio[i];
    if (gpio_pin != NOT_USED) {
      gpio_init(gpio_pin);
      gpio_set_dir(gpio_pin, GPIO_OUT);
      gpio_put(gpio_pin, 0);
    }
  }

  for (int i = 0; i < NUM_INPUTS; i++) {    
    int gpio_pin = server_input_gpio[i];
    if (gpio_pin != NOT_USED) {
      gpio_init(gpio_pin);
      gpio_set_dir(gpio_pin, GPIO_IN);
    }
  }

  // set gpio interrupts
  gpio_set_irq_enabled_with_callback(SYSTEM_INTERLOCK_IN_GPIO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
  gpio_set_irq_enabled_with_callback(SAFETY_INTERLOCK_IN_GPIO, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
}

void init_server_adc() {
    adc_init(); 

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(26);  // ADC0
    adc_gpio_init(27);  // ADC1
    adc_gpio_init(28);  // ADC2

    // Select ADC input 0 (GPIO26)
    adc_select_input(0);
}

void init_server_timers() {
    // Set up the repeating timers.
    static struct repeating_timer sensor_read_timer;  // must be static
    add_repeating_timer_ms(-timer_period, read_sensor_timer_callback, NULL, &sensor_read_timer); 

    static struct repeating_timer blink_timer;
    add_repeating_timer_ms(-200, blink_timer_callback, NULL, &blink_timer);
}

void read_server_sensors() {  
    server_registers[PICO_ADC0_HOLDING_REG] = read_adc_channel(PICO_ADC0_HOLDING_REG);
    server_registers[PICO_ADC1_HOLDING_REG] = read_adc_channel(PICO_ADC1_HOLDING_REG);
    server_registers[PICO_ADC2_HOLDING_REG] = read_adc_channel(PICO_ADC2_HOLDING_REG);    

#if SENSOR_DEBUG
      if (timer_hits % TIMER_FREQUENCY_HZ == 0) {
        printf("FLOW METER FREQUENCY: %d\n", delta);
      }
#endif
}

bool check_sensor_thresholds() {
  bool interlock = GOOD;
  
  if (server_registers[PICO_ADC0_HOLDING_REG] > 0xFF0) {
    interlock = BAD;  
    printf("ADC0 threshold tripped\n");
  }

  if (server_registers[PICO_ADC1_HOLDING_REG] > 0xFF0) {
    interlock = BAD;
    printf("ADC1 threshold tripped\n");
  }

  if (server_registers[PICO_ADC2_HOLDING_REG] > 0xF00) {
    interlock = BAD;
    printf("ADC2 threshold tripped\n");
  }

  return interlock;
}
