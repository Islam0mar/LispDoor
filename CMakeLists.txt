cmake_minimum_required(VERSION 3.1)

# The name of the target operating system, before project def.
# ============================================================
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)
SET(CMAKE_CROSSCOMPILING 1)

# Project name
# ===========+
project(LISP_DOOR ASM C)

# Dependencies
# ============
set(MY_PROJECT_PATH ${CMAKE_CURRENT_SOURCE_DIR})


# The name of the target operating system
# =======================================
set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM     Generic)
set(CMAKE_SYSTEM_VERSION   1)
set(CMAKE_SYSTEM_PROCESSOR arm-eabi)
SET(CMAKE_CROSSCOMPILING 1)

# Which compiler to use for C and ASM
# ===================================
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER     arm-none-eabi-as)
set(CMAKE_LINKER           arm-none-eabi-ld)
set(CMAKE_OBJCOPY     	   arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP          arm-none-eabi-objdump)
set(CMAKE_SIZE             arm-none-eabi-size)
set(CMAKE_ASM_LINK_EXECUTABLE "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

set(CMAKE_EXE_LINKER_FLAGS "")

# Settings
# ============
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_VERBOSE_MAKEFILE 1)

# Configuration types
# ====================
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
IF(CMAKE_BUILD_TYPE)
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS  ${CMAKE_CONFIGURATION_TYPES})
ELSE()
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
  message(STATUS "Default build type: ${CMAKE_BUILD_TYPE}")
ENDIF()

# Additional compilation flags based on configuration type
# ========================================================
set(CMAKE_C_FLAGS_DEBUG "-ggdb3 -O0") 
set(CMAKE_C_FLAGS_RELEASE "-Ofast")
set(CMAKE_ASM_FLAGS_DEBUG "-ggdb3 ")
set(CMAKE_ASM_FLAGS_RELEASE "")

# Linker flags
# ============
set(LINKER_SCRIPT ${MY_PROJECT_PATH}/STM32F103C8T6.ld)
set(ASM_LINKER_FLAGS -v -T ${LINKER_SCRIPT})
set(C_LINKER_FLAGS -T ${LINKER_SCRIPT} -Wl,--gc-sections -Wl,-Map=my_program.map -mthumb -mcpu=cortex-m3 --specs=nosys.specs -Wl,--no-undefined -Wl,--no-allow-shlib-undefined)

# Versioning
# ==========
file(STRINGS "${MY_PROJECT_PATH}/src/config.h" version_defines
  REGEX "#define VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${version_defines})
  if(ver MATCHES "#define VERSION_(MAJOR|MINOR|PATCH) +([^ ]+)$")
    set(MY_PROJECT_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
  endif()
endforeach()
set(${PROJECT_NAME}_VERSION
  ${MY_PROJECT_VERSION_MAJOR}.${MY_PROJECT_VERSION_MINOR}.${MY_PROJECT_VERSION_PATCH})
message(STATUS "${PROJECT_NAME} version: v${${PROJECT_NAME}_VERSION}")

# Output
# ======
set(MY_TARGET "${PROJECT_NAME}")
add_executable(${MY_TARGET} ${LINKER_SCRIPT})
add_subdirectory(src)
target_sources(${MY_TARGET} PUBLIC
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c
  )
target_include_directories(${MY_TARGET} PUBLIC
  ${MY_PROJECT_PATH}/Drivers/STM32F1xx_HAL_Driver/Inc
  ${MY_PROJECT_PATH}/Drivers/CMSIS/Device/ST/STM32F1xx/Include
  ${MY_PROJECT_PATH}/Drivers/CMSIS/Include
  )
target_link_options(${MY_TARGET} PUBLIC
  $<$<COMPILE_LANGUAGE:ASM>:${ASM_LINKER_FLAGS}>
  $<$<COMPILE_LANGUAGE:C>:${C_LINKER_FLAGS}>
  )
target_compile_options(${MY_TARGET} PUBLIC
  $<$<COMPILE_LANGUAGE:ASM>:-mimplicit-it=always> 
  $<$<COMPILE_LANGUAGE:ASM>:-mthumb> 
  $<$<COMPILE_LANGUAGE:ASM>:-mcpu=cortex-m3> 
  $<$<COMPILE_LANGUAGE:C>:-std=c99>
  $<$<COMPILE_LANGUAGE:C>:-mthumb>
  $<$<COMPILE_LANGUAGE:C>:-save-temps>
  $<$<COMPILE_LANGUAGE:C>:-mcpu=cortex-m3>
  $<$<COMPILE_LANGUAGE:C>:-fno-builtin>
  $<$<COMPILE_LANGUAGE:C>:-DUSE_HAL_DRIVER>
  $<$<COMPILE_LANGUAGE:C>:-DSTM32F103xB>
  $<$<COMPILE_LANGUAGE:C>:-ffreestanding>
  $<$<COMPILE_LANGUAGE:C>:-fdata-sections>
  $<$<COMPILE_LANGUAGE:C>:-ffunction-sections>
  $<$<COMPILE_LANGUAGE:C>:-Wall>
  $<$<COMPILE_LANGUAGE:C>:-Wextra>
  $<$<COMPILE_LANGUAGE:C>:-Wcast-align>
  $<$<COMPILE_LANGUAGE:C>:-Wconversion>
  $<$<COMPILE_LANGUAGE:C>:-Wsign-conversion>
  $<$<COMPILE_LANGUAGE:C>:-Wshadow>
  $<$<COMPILE_LANGUAGE:C>:-Wlogical-op>
  $<$<COMPILE_LANGUAGE:C>:-Wsuggest-final-types>
  $<$<COMPILE_LANGUAGE:C>:-Wsuggest-final-methods>
  $<$<COMPILE_LANGUAGE:C>:-pedantic> 
  )
set_target_properties(${MY_TARGET} PROPERTIES
  OUTPUT_NAME ${MY_TARGET}
  SUFFIX ".elf" 
  ARCHIVE_OUTPUT_DIRECTORY "${MY_PROJECT_PATH}/${CMAKE_BUILD_TYPE}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${MY_PROJECT_PATH}/${CMAKE_BUILD_TYPE}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${MY_PROJECT_PATH}/${CMAKE_BUILD_TYPE}/bin"
  BINARY_OUTPUT_DIRECTORY "${MY_PROJECT_PATH}/${CMAKE_BUILD_TYPE}/bin"
  VERSION ${${PROJECT_NAME}_VERSION}
  INTERFACE_${MY_TARGET}_MAJOR_VERSION ${MY_PROJECT_VERSION_MAJOR}
  COMPATIBLE_INTERFACE_STRING ${MY_TARGET}_MAJOR_VERSION
  )
get_target_property(MY_TARGET_BINARY_OUTPUT_DIRECTORY ${MY_TARGET} BINARY_OUTPUT_DIRECTORY)
add_custom_command(TARGET ${MY_TARGET} POST_BUILD
  COMMAND ${CMAKE_OBJDUMP} -D $<TARGET_FILE:${MY_TARGET}> >${MY_TARGET}.list
  COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${MY_TARGET}> ${MY_TARGET}.hex
  COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${MY_TARGET}> ${MY_TARGET}.bin
  COMMAND ${CMAKE_SIZE} -A -x -t  $<TARGET_FILE:${MY_TARGET}>
  WORKING_DIRECTORY ${MY_TARGET_BINARY_OUTPUT_DIRECTORY}
  COMMENT "Building: ${MY_TARGET}.list ${MY_TARGET}.hex ${MY_TARGET}.bin"
  )
add_custom_target(UPLOAD
  st-flash write ${MY_TARGET}.bin 0x08000000
  WORKING_DIRECTORY ${MY_TARGET_BINARY_OUTPUT_DIRECTORY}
  COMMENT "UPLOADING..."
  )
add_custom_target(DEBUG
  arm-none-eabi-gdb -tui --command=attach.gdb ${MY_TARGET}.elf
  WORKING_DIRECTORY ${MY_TARGET_BINARY_OUTPUT_DIRECTORY}
  )
# upload
# /* st-flash write forth.bin 0x08000000 */
# openocd -f interface/stlink-v2.cfg -c "transport select hla_swd" -f target/stm32f1x.cfg

# debugg
# arm-none-eabi-gdb -tui --command=attach.gdb forth.elf
# i r "info register" , i r r1
# b
# clean 
# continue

# expand macros with -e flag
# generate asm from C/C++ 
# arm-none-eabi-gcc -S -fverbose-asm -Ofast -o os.s system_stm32f1xx.c -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include  -DSTM32F103xB -I../Drivers/CMSIS/Include -mthumb -mcpu=cortex-m3
# COMMAND ${CMAKE_C_COMPILER} -S  ${PROJECT_PATH}/src/main.c -fverbose-asm -Ofast -mthumb -mcpu=cortex-m3 -fno-builtin -Wall -std=c99 -ffreestanding -fdata-sections -ffunction-sections -o ${ASM_FILE} -I${PROJECT_PATH}/include
