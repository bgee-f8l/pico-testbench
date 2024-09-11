#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define HEARTBEAT_MS    200
#define HEARTBEAT_PAUSE 5

void init_heartbeat(uint16_t address);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif // HEARTBEAT_H