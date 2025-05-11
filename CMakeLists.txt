# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initializes the Raspberry Pi Pico SDK
include(pico_sdk_import.cmake)

project(lab01_galton_board-filipe19 C CXX ASM)

pico_sdk_init()

# Lista de arquivos fonte do projeto
# Adiciona o executável principal com todos os arquivos fonte
add_executable(lab01_galton_board-filipe19
    src/galton_display.c
    src/galton_simulation.c
    inc/ssd1306_i2c.c
)


pico_set_program_name(lab01_galton_board-filipe19 "lab01_galton_board-filipe19")
pico_set_program_version(lab01_galton_board-filipe19 "0.1")

# Habilita saída USB (para depuração)
pico_enable_stdio_usb(lab01_galton_board-filipe19 1)

# Bibliotecas padrão
target_link_libraries(lab01_galton_board-filipe19 
    pico_stdlib 
    hardware_i2c 
    hardware_adc 
    hardware_pwm 
    hardware_gpio 
    pico_time
)

# Inclui os diretórios necessários
target_include_directories(lab01_galton_board-filipe19 PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Gera arquivos adicionais (uf2, hex, etc)
pico_add_extra_outputs(lab01_galton_board-filipe19)

