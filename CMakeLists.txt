cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/../RF24/CMakeLists.txt)
include($ENV{PICO_SDK_PATH}/../RF24Network/CMakeLists.txt)
include($ENV{PICO_SDK_PATH}/../RF24Mesh/CMakeLists.txt)

project(Main)

pico_sdk_init()

add_executable(Main main.cpp)

# Link with the pico stdlib
target_link_libraries(
    Main
    pico_unique_id
    pico_stdlib
    pico_multicore
    hardware_adc
    hardware_spi
    RF24
    RF24Network
    RF24Mesh
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(Main)

# Enable USB serial
pico_enable_stdio_usb(Main 1)
pico_enable_stdio_uart(Main 1)