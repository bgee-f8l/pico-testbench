// RS485 UART configuration
#define RS485_UART_ID       uart1
#define RS485_UART_TX_PIN   MODBUS_TX_PIN
#define RS485_UART_RX_PIN   MODBUS_RX_PIN
#define RS485_UART_RTS_PIN  7

// UART configuration
#define BAUD_RATE           115200
#define DATA_BITS           8
#define STOP_BITS           1
#define PARITY              UART_PARITY_NONE

// GPIO macros
#define OFF   0
#define ON    1

// LED pin
#define LED_PIN             PICO_DEFAULT_LED_PIN

// Modbus write enable pin
//#define RS485_HALF_DUPLEX 
#define MODBUS_WRITE_ENABLE_PIN   26

// Buffer size
#define BUFFER_SIZE   256

// Function prototypes
void init_debug_uart();
void init_rs485_uart();
void rs485_send_data(const char *data);
int rs485_receive_data(char *buffer, uint32_t len);

// Main functions
void receive();
void hello_world();

// Helpers
bool is_valid_message(const char *message, int len);
void print_message_with_hex(const char *message, int len);

// Pico Unique ID
#define PICO_UNIQUE_ID 0x01
