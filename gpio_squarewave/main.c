/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define FREQUENCY_HZ    1
#define HALF_PERIOD_MS(freq)  (1000000 / freq / 2)

#define ADC0_CHANNEL    0
#define ADC0_GPIO       26
#define ADC_VREF        3.18

#define HZ_PER_BIT      0.061

#define USE_ADC_FREQUENCY
#define SENSOR_DEBUG

// DEBUG UART configuration
#define DEBUG_UART_ID           uart0
#define DEBUG_TX_PIN            0
#define DEBUG_RX_PIN            1
#define DEBUG_BAUD_RATE         921600
#define DATA_BITS               8
#define STOP_BITS               1
#define PARITY                  UART_PARITY_NONE

bool one_second_flag = false;

// setup one_second callback
bool one_second_callback() {    
    one_second_flag = true;   

    return true;
}

uint16_t read_adc_channel(int adc_channel) {
    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    adc_select_input(adc_channel);
    uint16_t result = adc_read();

#ifdef SENSOR_DEBUG
  if (one_second_flag) {
    const float conversion_factor = ADC_VREF / (1 << 12);
    printf("Raw value: 0x%03x, voltage: %f V\n", result, result * conversion_factor);
  }
#endif

    return result;
}

int main() {

    // Initialize the debug UART
    stdio_uart_init_full(DEBUG_UART_ID, DEBUG_BAUD_RATE, DEBUG_TX_PIN, DEBUG_RX_PIN);
    uart_set_hw_flow(DEBUG_UART_ID, false, false);
    uart_set_format(DEBUG_UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // set up gpio
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    const uint GPIO_SQUAREWAVE = 6;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(GPIO_SQUAREWAVE);
    gpio_set_dir(GPIO_SQUAREWAVE, GPIO_OUT);

    // setup adc
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(ADC0_GPIO);  // ADC0

    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    // initialize frequency
    uint16_t frequency = FREQUENCY_HZ;

    // Set up the repeating timer.
    static struct repeating_timer one_second_timer;  // must be static
    add_repeating_timer_ms(-1000, one_second_callback, NULL, &one_second_timer); 

    while (true) {
        gpio_put(LED_PIN, 1);
        gpio_put(GPIO_SQUAREWAVE, 1);
        sleep_us(HALF_PERIOD_MS(frequency));
        //sleep_ms(HALF_PERIOD_MS(frequency));
        gpio_put(LED_PIN, 0);
        gpio_put(GPIO_SQUAREWAVE, 0);
        sleep_us(HALF_PERIOD_MS(frequency));
        //sleep_ms(HALF_PERIOD_MS(frequency));

#ifdef USE_ADC_FREQUENCY
        if (one_second_flag) {
            // read adc and recaculate frequency
            uint16_t value = read_adc_channel(ADC0_CHANNEL);            
            
            frequency = value * HZ_PER_BIT;
            //guard against low frequencies
            if (frequency < 5) {
                frequency = 5;
            }

            one_second_flag = false;

            printf("Square Wave frequency: %d, period: %d\n", frequency, (uint16_t) HALF_PERIOD_MS(frequency));
        }
#endif
    }
}