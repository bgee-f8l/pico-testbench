#ifndef I2C_MULTICORE_H
#define I2C_MULTICORE_H

#ifdef __cplusplus
extern "C" {
#endif

void i2c_write_multicore(i2c_inst_t *i2c_port, uint8_t addr, const uint8_t *src, size_t len, bool nostop);

uint32_t i2c_read_multicore(i2c_inst_t *i2c_port, uint8_t addr, uint8_t *dst, size_t len, bool nostop);

void init_i2c_multicore(i2c_inst_t *i2c_port, uint sda_gpio, uint scl_gpio);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif // I2C_MULTICORE_H