
## Dependencies ###################
CPMAddPackage("gh:bfgroup/Lyra#1.5.1")
CPMAddPackage("gh:ArthurSonzogni/FTXUI@3.0.0")
#CPMAddPackage("gh:biojppm/rapidyaml@0.5.0")
CPMAddPackage(
        GITHUB_REPOSITORY "biojppm/rapidyaml"
        VERSION 0.5.0
        OPTIONS "RYML_WITH_TAB_TOKENS ON"
)
CPMAddPackage("gh:jbeder/yaml-cpp#0.8.0")



## Cli Tool #######################
file(GLOB_RECURSE canSotCli_headers CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/**/*.h"
        "${CMAKE_CURRENT_LIST_DIR}/src/*.hpp" "${CMAKE_CURRENT_LIST_DIR}/src/**/*.hpp"
        )
file(GLOB_RECURSE canSotCli_src CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp" "${CMAKE_CURRENT_LIST_DIR}/src/**/*.cpp"
        )

#add_library(${PROJECT_NAME}-baseLib INTERFACE ${baseLibHeaders} ${baseLibSrc})
add_executable(canSotCli ${canSotCli_headers} ${canSotCli_src})
set_target_properties(canSotCli PROPERTIES CXX_STANDARD 20)
target_include_directories(${PROJECT_NAME}-baseLib PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/src/canSotCliTool/>
        )
target_link_libraries(canSotCli
        PRIVATE lyra ryml::ryml yaml-cpp
        PRIVATE ftxui::screen ftxui::dom ftxui::component
        )
