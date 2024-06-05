#create executables
# Define the executable
add_executable(pio-counter
    main.c    
)       

# generate C header for the PIO program
pico_generate_pio_header(pio-counter ${CMAKE_CURRENT_LIST_DIR}/transition_counter.pio)

# pull in common dependencies and additional i2c hardware support
target_link_libraries(pio-counter pico_stdlib hardware_gpio hardware_pio)

# create uf2 file etc.
pico_add_uf2_output(pio-counter)
