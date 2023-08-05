cmake_minimum_required(VERSION 3.19)

## Dependencies ###################
## Toolchain for stm32
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/CPM.cmake)
CPMAddPackage("gh:ObKo/stm32-cmake@2.1.0")
set(CMAKE_TOOLCHAIN_FILE ${CPM_PACKAGE_stm32-cmake_SOURCE_DIR}/cmake/stm32_gcc.cmake)
list(APPEND CMAKE_MODULE_PATH "${CPM_PACKAGE_stm32-cmake_SOURCE_DIR}/cmake/")



set(CMAKE_CXX_STANDARD 20)
project(
        CanSotProtocol-ExampleClientStm32
        VERSION 0.0
        LANGUAGES CXX C ASM # ASM needed for compiling startup file
)

## Dependencies ###################
stm32_fetch_cube(F1)
stm32_fetch_hal(F1)
stm32_fetch_cmsis(F1)

find_package(CMSIS COMPONENTS STM32F1 REQUIRED)
find_package(HAL COMPONENTS STM32F1 REQUIRED)

## add SOT Can lib
include(ExternalProject)

FetchContent_Declare(
        CanSotLib
        SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../"
        DOWNLOAD_COMMAND ""
        #SOURCE_SUBDIR "CanSotLib"
)
FetchContent_MakeAvailable(CanSotLib)

#include(../../CMakeLists.txt)



# Example client application ####
file(GLOB_RECURSE exampleClientStm32Src CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/**/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/**/*.c"
        )
add_executable(${PROJECT_NAME}-example-client-stm32 ${exampleClientStm32Src})
message(STATUS "SOURCE_FILES = ${exampleClientStm32Src}")
#target_compile_features(${PROJECT_NAME}-baseLib-tests PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}-example-client-stm32  PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cubeMxGenerated/Inc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cubeMxGenerated/USB_DEVICE/App"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/cubeMxGenerated/USB_DEVICE/Target"
        )
target_link_libraries(${PROJECT_NAME}-example-client-stm32   PRIVATE
        #CanSotLib::CanSotProtocol-baseLib
        CanSotProtocol-baseLib
        HAL::STM32::F1
        HAL::STM32::F1::I2C
        HAL::STM32::F1::SPI
        HAL::STM32::F1::ADC
        HAL::STM32::F1::RCCEx
        HAL::STM32::F1::CAN
        HAL::STM32::F1::PCD
        HAL::STM32::F1::LL_USB
        HAL::STM32::F1::GPIO
        HAL::STM32::F1::CORTEX
        HAL::STM32::F1::TIMEx
        HAL::STM32::F1::DMA
        CMSIS::STM32::F1
        CMSIS::STM32::F103C8
        STM32::NoSys
        )

#stm32_get_devices_by_family(STM_DEVICES FAMILY F1)
#message(STATUS "STM_DEVICES ${STM_DEVICES}")
#stm32_get_chip_type(F1 03 TYPE)
#stm32_get_memory_info(FAMILY F1 DEVICE F103C8)
stm32_print_size_of_target(${PROJECT_NAME}-example-client-stm32)
stm32_generate_binary_file(${PROJECT_NAME}-example-client-stm32)

#stm32_add_linker_script(${PROJECT_NAME}-example-client-stm32 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/cubeMxGenerated/Startup/STM32F103C8TX_FLASH.ld)