cmake_minimum_required(VERSION 3.10)

project(CMakeTest VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(../Hydrolibs Hydrolib)

file(GLOB SOURCES src/*.cpp)
add_executable(TTY_reader ${SOURCES})

target_link_libraries(TTY_reader HydrolibSerialProtocol)

target_include_directories(TTY_reader PUBLIC include)

target_compile_features(TTY_reader PUBLIC cxx_std_20)
target_compile_options(TTY_reader PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
    -Woverloaded-virtual -Wsuggest-override)
