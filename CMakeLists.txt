cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(placeholder-fc C CXX)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB code_SRC
     "src/*.h"
     "src/*.c"
)

add_executable(code
	${code_SRC}
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(code pico_stdlib hardware_i2c)

# enable usb output, disable uart output
pico_enable_stdio_usb(code 1)
pico_enable_stdio_uart(code 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(code)
