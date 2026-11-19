cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME BeaconGalaxy)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Add touchgfx generated sources and libraries
add_subdirectory(cmake/touchgfx)

# ---------------------------------------------------------------------------
# BeaconGalaxy application code (App/) and code shared with AudioGalaxy
# (common/). CubeMX owns Core/, Drivers/, Middlewares/, FATFS/.
# ---------------------------------------------------------------------------
# Shared code and tools: inside this project (stand-alone repository) or next
# to it (the combined Galaxy workspace).
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/common)
    set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/common)
    set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
else()
    set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
    set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tools)
endif()

# The app is linked at 0x08040000, after the GalaxyBoot bootloader (256 KB).
# BeaconGalaxy_app.ld is the CubeMX script with only the FLASH region moved.
string(REPLACE "STM32F405XX_FLASH.ld" "BeaconGalaxy_app.ld" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
target_compile_definitions(stm32cubemx INTERFACE USER_VECT_TAB_ADDRESS VECT_TAB_OFFSET=0x00040000U)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/App/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/App/*.cpp)
set(COMMON_SOURCES
    ${COMMON_DIR}/cli/cli.c
    ${COMMON_DIR}/cli/cli_parse.c
    ${COMMON_DIR}/cli/log.c
    ${COMMON_DIR}/util/crc16.c
    ${COMMON_DIR}/util/crc32.c
    ${COMMON_DIR}/util/swd_console.c
    ${COMMON_DIR}/link/galaxy_link.c
    ${COMMON_DIR}/stm32f4/uart_io.c
    ${COMMON_DIR}/stm32f4/watchdog.c
    ${COMMON_DIR}/galaxy/oled.c
    ${COMMON_DIR}/galaxy/eeprom.c)
set_source_files_properties(${COMMON_DIR}/galaxy/oled.c ${COMMON_DIR}/galaxy/eeprom.c
    PROPERTIES COMPILE_DEFINITIONS GALAXY_RTOS=1)

# TouchGFX target code we own (outside target/generated, kept by code generation)
set(GUI_TARGET_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/TouchGFX/target/GalaxyButtonController.cpp)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${APP_SOURCES} ${COMMON_SOURCES} ${GUI_TARGET_SOURCES})

# Own code: extra warnings (CI fails on any warning in App/ or common/)
set_source_files_properties(${APP_SOURCES} ${COMMON_SOURCES} PROPERTIES COMPILE_OPTIONS "-Wextra")

# Add include paths (the TouchGFX GUI library needs the App headers too:
# the GUI reaches the application through App/Ui/ui_bridge.h)
set(APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Main
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Console
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Board
    ${COMMON_DIR}/galaxy
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Storage
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Link
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Settings
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Gif
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Ui
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Matrix
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Fx
    ${CMAKE_CURRENT_SOURCE_DIR}/App/Eq
    ${COMMON_DIR}/cli
    ${COMMON_DIR}/util
    ${COMMON_DIR}/link
    ${COMMON_DIR}/stm32f4
)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(TouchGFX PRIVATE ${APP_INCLUDE_DIRS})

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

# Update file for GalaxyBoot: raw binary + CRC trailer -> build/<preset>/BeaconGalaxy.bin
# (copy it to the SD card root, then About > UPGRADE; see GalaxyBoot/App/Boot/image.h)
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${CMAKE_PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR}/BeaconGalaxy_raw.bin
        COMMAND ${Python3_EXECUTABLE} ${TOOLS_DIR}/append_crc.py
                ${CMAKE_CURRENT_BINARY_DIR}/BeaconGalaxy_raw.bin ${CMAKE_CURRENT_BINARY_DIR}/BeaconGalaxy.bin
        VERBATIM)
else()
    message(WARNING "Python 3 not found: BeaconGalaxy.bin (SD-card update file) is not made")
endif()
