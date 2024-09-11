/**
 * Multicore safe i2c driver
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/mutex.h"
#include "hardware/i2c.h"

mutex_t i2c_mutex;

// void init_i2c_multicore() {
//     mutex_init(&i2c_mutex);
// }

void i2c_write_multicore(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop) {
    // Lock the mutex to prevent other cores from accessing the I2C hardware
    mutex_enter_blocking(&i2c_mutex);
    
    // Perform the I2C write operation
    i2c_write_blocking(i2c, addr, src, len, nostop);
    
    // Unlock the mutex after the operation is complete
    mutex_exit(&i2c_mutex);
}

uint32_t i2c_read_multicore(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop) {
    // Lock the mutex to prevent other cores from accessing the I2C hardware
    mutex_enter_blocking(&i2c_mutex);
    
    // Perform the I2C write operation
    uint32_t ret;
    ret = i2c_read_blocking(i2c, addr, dst, len, nostop);
    
    // Unlock the mutex after the operation is complete
    mutex_exit(&i2c_mutex);

    return ret;
}

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
static bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void init_i2c_multicore(i2c_inst_t *i2c_port, const uint sda_gpio, const uint scl_gpio) {
//   #if !defined(i2c0) || !defined(I2C_SDA_GPIO) || !defined(I2C_SCL_GPIO)
//   #warning i2c/bus_scan example requires a board with I2C pins
//       puts("Default I2C pins were not defined");
//   #else
    mutex_init(&i2c_mutex);

    i2c_init(i2c_port, 400 * 1000);
    gpio_set_function(sda_gpio, GPIO_FUNC_I2C);
    gpio_set_function(scl_gpio, GPIO_FUNC_I2C);
    gpio_pull_up(sda_gpio);
    gpio_pull_up(scl_gpio);
    // Make the I2C pins available to picotool
    //bi_decl(bi_2pins_with_func(sda_gpio, scl_gpio, GPIO_FUNC_I2C));

    printf("I2C Bus Scan\n\r");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n\r");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_multicore(i2c_port, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
    // #endif
}