cmake_minimum_required(VERSION 3.16)
project(ChroniquesDeLaFaille LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(BUILD_GRAPHICAL_GAME "Build the Raylib desktop game" ON)

file(GLOB_RECURSE GAME_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/characters/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/entities/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/systems/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp")

add_library(game_core ${GAME_CORE_SOURCES})
target_include_directories(game_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/characters"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/entities"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/systems")

if(MSVC)
    target_compile_options(game_core PRIVATE /W4)
else()
    target_compile_options(game_core PRIVATE -Wall -Wextra -pedantic)
endif()

add_executable(chroniques_de_la_faille_terminal src/terminal_main.cpp)
target_link_libraries(chroniques_de_la_faille_terminal PRIVATE game_core)

if(BUILD_GRAPHICAL_GAME)
    find_package(raylib 5.5 CONFIG REQUIRED)
    add_executable(chroniques_de_la_faille main.cpp src/ui/RaylibGame.cpp)
    target_link_libraries(chroniques_de_la_faille PRIVATE game_core raylib)
    if(MSVC)
        target_compile_options(chroniques_de_la_faille PRIVATE /W4)
    else()
        target_compile_options(chroniques_de_la_faille PRIVATE -Wall -Wextra -pedantic)
    endif()
endif()

enable_testing()
add_executable(roguelite_tests tests/roguelite_tests.cpp)
target_link_libraries(roguelite_tests PRIVATE game_core)
add_test(NAME roguelite_tests COMMAND roguelite_tests)
set_tests_properties(roguelite_tests PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
