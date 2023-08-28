cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
project(
        CanSotProtocol
        VERSION 0.0
        LANGUAGES CXX
)


## Dependencies ###################
include(${CMAKE_CURRENT_LIST_DIR}/cmake/CPM.cmake)

CPMAddPackage("gh:doctest/doctest@2.4.11")
CPMAddPackage("gh:ETLCPP/etl#20.37.2")
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")



## Base Lib #######################
file(GLOB_RECURSE baseLibHeaders CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/baseLib/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/baseLib/**/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/baseLib/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/src/baseLib/**/*.hpp"
        )
file(GLOB_RECURSE baseLibSrc CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/baseLib/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/baseLib/**/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/src/driver/**/*.cpp"
        )
message(STATUS "baseLibSrc: ${baseLibSrc}")

#add_library(${PROJECT_NAME}-baseLib INTERFACE ${baseLibHeaders} ${baseLibSrc})
add_library(${PROJECT_NAME}-baseLib ${baseLibHeaders} ${baseLibSrc})
set_target_properties(${PROJECT_NAME}-baseLib PROPERTIES CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME}-baseLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/baseLib/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/driver/>
        )
target_link_libraries(${PROJECT_NAME}-baseLib PUBLIC etl::etl)

## just for dev, so that we get nice code completion for some templates
# !! deactivate for using the library !!
#target_compile_definitions(${PROJECT_NAME}-baseLib PRIVATE DEV_MODE)


# Test for Base lib ####
file(GLOB_RECURSE baseLibTestSrc CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/tests/baseLib/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/tests/baseLib/**/*.cpp")
add_executable(${PROJECT_NAME}-baseLib-tests ${baseLibHeaders} ${baseLibSrc} ${baseLibTestSrc})
#target_compile_features(${PROJECT_NAME}-baseLib-tests PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}-baseLib-tests PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/src/baseLib/
        ${CMAKE_CURRENT_LIST_DIR}/src/driver
        "${CMAKE_CURRENT_LIST_DIR}/tests/testUtil/"
        )
target_link_libraries(${PROJECT_NAME}-baseLib-tests PRIVATE etl::etl doctest::doctest )



## Cli Tool ######
include(${CMAKE_CURRENT_LIST_DIR}/cliTool/CMakeLists.txt)



## Example applications ######
include(${CMAKE_CURRENT_LIST_DIR}/example/master-linux/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/example/client-linux/CMakeLists.txt)
#include(example/client-stm32-hal/CMakeLists.txt)



#[[
packageProject(
        NAME ${PROJECT_NAME}-baseLib
        VERSION ${PROJECT_VERSION}
        NAMESPACE ${PROJECT_NAME}
        BINARY_DIR ${PROJECT_BINARY_DIR}
        INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
        INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
        DEPENDENCIES "etl"
)
]]#