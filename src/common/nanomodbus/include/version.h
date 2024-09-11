#ifndef VERSION_H
#define VERSION_H

// Track version of this code
#define DEFAULT_VERSION  (uint16_t)1

#define COMMIT_SHORT_HASH 0xDEADBEEF

// define reserved holding registers
#define VERSION_NUMBER_REG          0x00       // (uint16_t) Version number
#define COMMIT_SHORT_HASH0_REG      0x01       // 0xDEAD
#define COMMIT_SHORT_HASH1_REG      0x02       // 0xBEEF
#define HARDWARE_VERSION_REG        0x03       // (uint16_t) Hardware version
#define ENABLE_DEBUG_REG            0x04       // Used to enable/disable debug output
#define RESERVED_2_REG              0x05
#define RESERVED_3_REG              0x06
#define RESERVED_4_REG              0x07
#define RESERVED_5_REG              0x08
#define RESERVED_6_REG              0x09
#define RESERVED_7_REG              0x0A
#define RESERVED_8_REG              0x0B
#define RESERVED_9_REG              0x0C
#define RESERVED_10_REG             0x0D
#define RESERVED_11_REG             0x0E
#define RESERVED_12_REG             0x0F

#define LAST_RESERVED_REG_ADDR      0x0F

#ifdef __cplusplus
}    // extern "C"
#endif

#endif    //VERSION_H
