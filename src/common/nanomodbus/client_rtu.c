/*
   This example client application connects via RTU to a server and sends some requests to it.
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "nanomodbus.h"
#include "rs485.h"

// The server address
#define RTU_SERVER_ADDRESS 1

// Debug flag
#define DEBUG false

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

void on_error()
{
    // Make the LED blink on error
    while (true)
    {
        gpio_put(LED_PIN, ON);
        sleep_ms(250);
        gpio_put(LED_PIN, OFF);
        sleep_ms(250);
    }
}

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

    // Initialize write enable pin
#ifdef RS485_HALF_DUPLEX
    gpio_init(MODBUS_WRITE_ENABLE_PIN);
    gpio_set_dir(MODBUS_WRITE_ENABLE_PIN, GPIO_IN);
#endif
}

int main()
{
    // Setup the board
    setup();
    
    // Initialize the nanomodbus client
    nmbs_platform_conf platform_conf;
    platform_conf.transport = NMBS_TRANSPORT_RTU;
    platform_conf.read = read_serial;
    platform_conf.write = write_serial;

    nmbs_t nmbs;
    nmbs_error err = nmbs_client_create(&nmbs, &platform_conf);
    if (err != NMBS_ERROR_NONE)
        on_error();

    nmbs_set_read_timeout(&nmbs, 1000);
    nmbs_set_byte_timeout(&nmbs, 100);

    nmbs_set_destination_rtu_address(&nmbs, RTU_SERVER_ADDRESS);

    nmbs_bitfield coils = {0};

    // This loop repeatedly sends a read request to the modbus server (for testing purposes)
    while (true) {
    #ifdef RS485_HALF_DUPLEX
        // Use the write enable pin (pin 26) to enable/disable writing to the modbus server        
        int write_enable = gpio_get(MODBUS_WRITE_ENABLE_PIN);
    #else
        int write_enable = 0;
    #endif

        if (write_enable == 0) {
            continue;
        }

        // Example read request - use whatever function you need
        printf("Reading 1 coil from address 64\n");

        nmbs_bitfield_reset(coils); // Reset whole bitfield to zero
        err = nmbs_read_coils(&nmbs, 64, 1, coils);
        if (err != NMBS_ERROR_NONE) {
            printf("Error reading 3 coils from address 64\n");
            // on_error();
        }

        sleep_ms(100);
    }

    // On success, keep the led on
    gpio_put(LED_PIN, ON);

    // No need to destroy the nmbs instance, terminate the program
    return 0;
}
