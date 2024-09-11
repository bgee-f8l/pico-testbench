#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "rs485.h"

int main() {

    // Initialize the UARTs
    init_debug_uart();
    init_rs485_uart();

    receive();
    
    return 0;
}

void receive() {
    int num_fails = 0;
    char buffer[BUFFER_SIZE];
    while (1) {
        // Wait for data to be received
        while (!uart_is_readable(RS485_UART_ID))
            tight_loop_contents();

        int received = rs485_receive_data(buffer, BUFFER_SIZE);

        if (received > 0) {
            // Echo back the received data
            printf("Echoing data: %s\n", buffer);
            rs485_send_data(buffer);  
        } else {
            num_fails++;

            if (num_fails >= 5) {
                // Send a message to the other Pico
                rs485_send_data("Plz work ^_^\n\r");

                num_fails = 0;
            }
        }

        sleep_ms(500);
    }
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

int rs485_receive_data(char *buffer, uint32_t len) {
    // Check if the buffer is valid
    if (buffer == NULL || len == 0) {
        return 0;
    }

    int received = 0;

    // Receive data from UART
    for (uint32_t i = 0; i < len; ++i) {
        if (uart_is_readable(RS485_UART_ID)) {
            buffer[received++] = uart_getc(RS485_UART_ID);
        } else {
            break;
        }
    }

    // Print the received data
    if (is_valid_message(buffer, received)) {
        printf("Received valid message: ");
        print_message_with_hex(buffer, received);
    } else {
        printf("Received invalid message: ");
        print_message_with_hex(buffer, received);
        received = 0;
    }
    
    return received;
}

bool is_valid_message(const char *message, int len) {
    if (message == NULL) return false;

    // Check if the message is valid
    for (uint32_t i = 0; i < len; ++i) {
        if (message[i] < 1 || message[i] > 127) {
            return false;
        }
    }

    return true;
}

void print_message_with_hex(const char *message, int len) {
    if (message == NULL) return;

    printf("%s ( ", message);
    // Print the message with hex values
    for (uint32_t i = 0; i < len; ++i) {
        printf("0x%02X ", message[i]);
    }

    printf(")\n");
}