#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/timer.h"
#include "heartbeat.h"

uint16_t blink_beats = 0;
uint16_t beat_count = 0;
bool blink = false;

bool heartbeat_timer_callback() { 
    blink = !blink;   
    gpio_put(PICO_DEFAULT_LED_PIN, blink);
    beat_count++;
    if (beat_count >= (2 * blink_beats)) {
        // we've reached the end, pause blinks for 1 second and then repeat
        blink = true;  // it'll get flipped on the gpio put.
        if (beat_count >= (2 * blink_beats + HEARTBEAT_PAUSE)) {
            // we've completed the blink cycle.  reset and start over
            blink = false;
            beat_count = 0;
        }
    }

    return true;
}

void init_heartbeat_timer() {
    static struct repeating_timer heartbeat_timer;
    add_repeating_timer_ms(-HEARTBEAT_MS, heartbeat_timer_callback, NULL, &heartbeat_timer);
}

// initalize the heartbeat using the modbus address
void init_heartbeat(uint16_t address) {
    blink_beats = address;
    init_heartbeat_timer();
}