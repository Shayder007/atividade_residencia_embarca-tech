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
# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Inicialização padrão para o VS Code Pico SDK
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa o SDK da Raspberry Pi Pico
include(pico_sdk_import.cmake)

project(luminosidade C CXX ASM)

# Inicializa o SDK
pico_sdk_init()

# Adiciona o executável principal
add_executable(luminosidade sensor_de_luminosidade.c)

# Nome e versão do programa
pico_set_program_name(luminosidade "luminosidade")
pico_set_program_version(luminosidade "0.1")

# Habilita saída USB e desabilita UART
pico_enable_stdio_uart(luminosidade 0)
pico_enable_stdio_usb(luminosidade 1)

# Vincula as bibliotecas necessárias
target_link_libraries(luminosidade
    pico_stdlib
    hardware_pwm
    hardware_adc
    hardware_i2c
)

# Adiciona os diretórios de include
target_include_directories(luminosidade PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Gera os arquivos extras como .uf2
pico_add_extra_outputs(luminosidade)


