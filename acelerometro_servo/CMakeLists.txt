# Generated Cmake Pico project file

# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Acelerometro_Servo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable
add_executable(Acelerometro_Servo Acelerometro_Servo.c inc/ssd1306_i2c.c)

pico_set_program_name(Acelerometro_Servo "Acelerometro_Servo")
pico_set_program_version(Acelerometro_Servo "0.1")

# Enable stdio over USB, disable over UART
pico_enable_stdio_uart(Acelerometro_Servo 0)
pico_enable_stdio_usb(Acelerometro_Servo 1)

# Add the standard library
target_link_libraries(Acelerometro_Servo pico_stdlib)

# Include paths
target_include_directories(Acelerometro_Servo PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Link required libraries
target_link_libraries(Acelerometro_Servo 
        hardware_i2c
        hardware_pwm
        hardware_adc
)

# Create binary, hex, uf2 etc.
pico_add_extra_outputs(Acelerometro_Servo)


