cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)


## Dependencies ###################
CPMAddPackage("gh:bfgroup/Lyra#1.6.1")


# Example client application ####
file(GLOB_RECURSE exampleClientLinuxSrc CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/**/*.cpp")
add_executable(${PROJECT_NAME}-example-client-linux ${exampleClientLinuxSrc})
target_link_libraries(${PROJECT_NAME}-example-client-linux  PRIVATE ${PROJECT_NAME}-baseLib lyra)
