#include <stdio.h>
#include "pico/stdlib.h"
#include "rs485.h"

int main() {

    // Initialize the UARTs
    init_debug_uart();
    init_rs485_uart();

    // Send initial message
    while (true) {
        absolute_time_t time = get_absolute_time();
        printf("Time: %lld\n", time);
        
        sleep_us(1e6);
    }
    
    return 0;
}

void hello_world() {

    while (true) {
        // char * message = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. In mollis nunc sed id semper risus in hendrerit gravida. Morbi tristique senectus et netus et. Neque convallis a cras semper auctor neque. Cursus vitae congue mauris rhoncus. Consectetur lorem donec massa sapien faucibus et. Cursus in hac habitasse platea dictumst. In eu mi bibendum neque. Massa sapien faucibus et molestie ac feugiat. Eleifend mi in nulla posuere. Aenean sed adipiscing diam donec adipiscing tristique risus. Egestas tellus rutrum tellus pellentesque eu tincidunt tortor aliquam. Nisl tincidunt eget nullam non nisi est. Nunc non blandit massa enim nec dui nunc mattis enim. Elit ut aliquam purus sit amet. Ornare suspendisse sed nisi lacus. Tempus quam pellentesque nec nam aliquam sem. Eleifend quam adipiscing vitae proin sagittis nisl rhoncus. Eget nunc lobortis mattis aliquam. Ac turpis egestas maecenas pharetra convallis.\n\nSit amet volutpat consequat mauris. Diam quam nulla porttitor massa id neque aliquam. Senectus et netus et malesuada fames ac turpis egestas. Elementum integer enim neque volutpat ac tincidunt vitae. Massa id neque aliquam vestibulum morbi blandit. Et sollicitudin ac orci phasellus egestas tellus rutrum. Orci phasellus egestas tellus rutrum tellus pellentesque eu tincidunt tortor. Pellentesque massa placerat duis ultricies lacus sed turpis. Neque ornare aenean euismod elementum nisi quis eleifend quam. Pulvinar neque laoreet suspendisse interdum consectetur libero id.\n\nPellentesque habitant morbi tristique senectus et netus. Consequat nisl vel pretium lectus quam id. Nibh ipsum consequat nisl vel pretium lectus quam id leo. Vivamus arcu felis bibendum ut. Vitae proin sagittis nisl rhoncus mattis rhoncus urna. Turpis egestas maecenas pharetra convallis. Luctus accumsan tortor posuere ac ut consequat. Integer malesuada nunc vel risus commodo viverra maecenas. Tellus molestie nunc non blandit massa. Velit euismod in pellentesque massa placerat duis. Sit amet volutpat consequat mauris nunc congue nisi vitae suscipit. Massa eget egestas purus viverra accumsan.\n\nAt varius vel pharetra vel turpis nunc eget lorem. Massa id neque aliquam vestibulum. Ullamcorper a lacus vestibulum sed arcu non. Nulla porttitor massa id neque aliquam vestibulum morbi blandit. Justo donec enim diam vulputate ut pharetra sit amet aliquam. Amet mauris commodo quis imperdiet massa. Enim eu turpis egestas pretium aenean pharetra magna. Dolor sit amet consectetur adipiscing elit ut. Risus viverra adipiscing at in tellus integer feugiat scelerisque varius. Urna cursus eget nunc scelerisque viverra mauris in. Amet consectetur adipiscing elit duis tristique sollicitudin nibh. Fringilla ut morbi tincidunt augue interdum velit euismod in pellentesque. Pretium quam vulputate dignissim suspendisse in est. Mauris a diam maecenas sed enim.\n\nTincidunt augue interdum velit euismod in pellentesque. Auctor augue mauris augue neque gravida in fermentum et. Mauris cursus mattis molestie a iaculis at. Aliquet nec ullamcorper sit amet risus nullam eget felis eget. Commodo nulla facilisi nullam vehicula ipsum. Consequat interdum varius sit amet mattis vulputate enim. Semper viverra nam libero justo laoreet sit amet cursus sit. Mauris rhoncus aenean vel elit. Eu facilisis sed odio morbi quis commodo odio aenean sed. Montes nascetur ridiculus mus mauris vitae. Mauris ultrices eros in cursus. Lorem ipsum dolor sit amet consectetur adipiscing. Purus semper eget duis at tellus at. Lacus viverra vitae congue eu consequat. Odio facilisis mauris sit amet massa vitae tortor condimentum. Tempus egestas sed sed risus pretium. Nulla malesuada pellentesque elit eget gravida cum sociis. Est ullamcorper eget nulla facilisi etiam. Dui faucibus in ornare quam viverra orci sagittis. Nunc aliquet bibendum enim facilisis gravida neque convallis a cras.\n\r";

        char * message = "Hello World\n\r";

        rs485_send_data(message);

        sleep_ms(1000);
    }

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