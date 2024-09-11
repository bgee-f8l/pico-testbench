#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "rs485.h"

void setup()
{
    // Initialize the builtin LED (used for error indication)
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, OFF);

    // Initialize the debug UART
    stdio_uart_init_full(DEBUG_UART_ID, BAUD_RATE, DEBUG_TX_PIN, DEBUG_RX_PIN);
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
}

int main() {

    // Initialize the UARTs
    setup();

    hello_world();
    
    return 0;
}

void hello_world() {

    while (true) {
        char * message = "Hello World\n\r";

        gpio_put(LED_PIN, OFF);
        rs485_send_data(message);
        gpio_put(LED_PIN, ON);

        sleep_ms(100);
    }

}

void rs485_send_data(const char *data) {
    if (data == NULL) return;

    // Set RTS to high (transmit enabled)
    gpio_put(RS485_UART_RTS_PIN, ON);

    // Send data
    printf("Sending data: %s\n", data);

    uart_puts(RS485_UART_ID, data);

    uart_tx_wait_blocking(RS485_UART_ID);
 
    // Set RTS to low (receive enabled)
    gpio_put(RS485_UART_RTS_PIN, OFF);
}

void init_debug_uart() {
    // Initialize the debug UART
    stdio_uart_init_full(DEBUG_UART_ID, BAUD_RATE, DEBUG_TX_PIN, DEBUG_RX_PIN);
    uart_set_hw_flow(DEBUG_UART_ID, false, false);
    uart_set_format(DEBUG_UART_ID, DATA_BITS, STOP_BITS, PARITY);
}

void init_rs485_uart() {
    // Initialize the RS485 UART
    uart_init(RS485_UART_ID, BAUD_RATE);

    gpio_set_function(RS485_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(RS485_UART_RX_PIN, GPIO_FUNC_UART);

    uart_set_hw_flow(RS485_UART_ID, false, false);
    uart_set_format(RS485_UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Initialize the RS485 RTS pin
    gpio_init(RS485_UART_RTS_PIN);
    gpio_set_dir(RS485_UART_RTS_PIN, GPIO_OUT);
    gpio_put(RS485_UART_RTS_PIN, OFF);  // Set RTS to low initially (receive enabled)
}
