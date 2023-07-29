cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)


## Dependencies ###################
CPMAddPackage("gh:bfgroup/Lyra#1.6.1")


# Example master application ####
file(GLOB_RECURSE exampleMasterLinuxSrc CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/**/*.cpp")
add_executable(${PROJECT_NAME}-example-master-linux ${exampleMasterLinuxSrc})
#target_compile_features(${PROJECT_NAME}-baseLib-tests PRIVATE cxx_std_17)
#target_include_directories(${PROJECT_NAME}-baseLib-tests PUBLIC src/baseLib/ "${CMAKE_CURRENT_SOURCE_DIR}/tests/testUtil/")
target_link_libraries(${PROJECT_NAME}-example-master-linux  PRIVATE ${PROJECT_NAME}-baseLib lyra)
