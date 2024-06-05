#create executables
# Define the executable
add_executable(pio-controlled-squarewave
    main.c
)       

# generate C header for the PIO program
pico_generate_pio_header(pio-controlled-squarewave ${CMAKE_CURRENT_LIST_DIR}/controlled-squarewave.pio)

# pull in common dependencies and additional i2c hardware support
target_link_libraries(pio-controlled-squarewave pico_stdlib hardware_gpio hardware_pio)

# create uf2 file etc.
pico_add_uf2_output(pio-controlled-squarewave)
