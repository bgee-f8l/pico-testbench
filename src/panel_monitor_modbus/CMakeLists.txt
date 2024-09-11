
# include directories
include_directories(include)
include_directories(../drivers)
include_directories(../common)
include_directories(../common/nanomodbus/include)


# Create modbus executables
add_executable(panel_monitor_server
        ${CMAKE_CURRENT_LIST_DIR}/../common/nanomodbus/nanomodbus.c       
        ${CMAKE_CURRENT_LIST_DIR}/../common/heartbeat/heartbeat.c      
        ${CMAKE_CURRENT_LIST_DIR}/../common/i2c_multicore/i2c_multicore.c
        ${CMAKE_CURRENT_LIST_DIR}/../common/usb_printf/usb_printf.c
        ${CMAKE_CURRENT_LIST_DIR}/../drivers/ina219.c
        ${CMAKE_CURRENT_LIST_DIR}/panel_monitor_modbus.c
        ${CMAKE_CURRENT_LIST_DIR}/panel_monitor_server_rtu.c
        )

# pull in common dependencies and additional i2c hardware support
target_link_libraries(panel_monitor_server pico_stdlib hardware_gpio hardware_adc hardware_timer hardware_i2c)

# enable usb uart for debug
pico_enable_stdio_usb(panel_monitor_server 1)

# create uf2 file etc.
pico_add_uf2_output(panel_monitor_server)