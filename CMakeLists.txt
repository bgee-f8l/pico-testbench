cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

include(pico_extras_import_optional.cmake)

project(pico_examples C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(example_auto_set_url.cmake)

# Set the CMP0079 policy to NEW to avoid the warning
if (POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

target_link_libraries(
	pico_stdlib
	INTERFACE pico_multicore
	)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        -Wno-maybe-uninitialized
        )

# Hardware-specific examples in subdirectories:
add_subdirectory(pio-squarewave)
add_subdirectory(gpio_squarewave)
add_subdirectory(frequency-counter)
add_subdirectory(pio-counter)
add_subdirectory(pio-blink)
add_subdirectory(pio-controlled-squarewave)
add_subdirectory(src/panel_monitor_modbus)
