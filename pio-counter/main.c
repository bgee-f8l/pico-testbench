#include <stdio.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "transition_counter.pio.h"

#define PIN_INPUT 15  // Update to use GPIO 15
#define PIN_INPUT2 14  // Update to use GPIO 15

// Function to reset the 32-bit counter
void reset_counter(PIO pio, uint sm) {
    pio_sm_exec(pio, sm, pio_encode_set(pio_x, 0));  // Set X to 0
}

int main() {
    stdio_init_all();

    // setup transition counter PIO
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &transition_counter_program);

    //setup first state machine
    uint sm0 = pio_claim_unused_sm(pio, true);
    transition_counter_program_init(pio, sm0, offset, PIN_INPUT);

    //initialize counts
    uint32_t count, delta = 0;
    uint32_t previous_count = 0xFFFFFFFF;        

    // setup second state machine
    uint sm1 = pio_claim_unused_sm(pio, true);
    transition_counter_program_init(pio, sm1, offset, PIN_INPUT2);

    uint32_t count2, delta2 = 0;
    uint32_t previous_count2 = 0xFFFFFFFF;     

    printf("Start transition counter\n");

    while (true) {
        // force push isr data into rx fifo
        pio_sm_exec(pio, sm0, pio_encode_jmp(4 + offset));          

        if (!pio_sm_is_rx_fifo_empty(pio, sm0)) {
            // Read the value of the ISR (32-bit counter) from the RX FIFO
            count = transition_counter_get_count(pio, sm0);            
            delta = previous_count - count;
            previous_count = count;

            // Print the counter value
            //printf("Low-to-high transitions: %u, delta: %u\n", count, delta);
            printf("1: %u, ", delta);
        }

        pio_sm_exec(pio, sm1, pio_encode_jmp(4 + offset));   

        if (!pio_sm_is_rx_fifo_empty(pio, sm1)) {
            // Read the value of the ISR (32-bit counter) from the RX FIFO
            count2 = transition_counter_get_count(pio, sm1);            
            delta2 = previous_count2 - count2;
            previous_count2 = count2;

            // Print the counter value
            //printf("Low-to-high transitions_2: %u, delta: %u\n", count2, delta2);     
            printf("2: %u\n", delta2);
        }
        
        sleep_ms(1000);
    }

    return 0;
}