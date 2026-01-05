cmake_minimum_required(VERSION 3.10)

project(CMakeTest VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES src/*.cpp)
add_executable(hydrosp ${SOURCES})

target_link_libraries(hydrosp HydrolibBusDatalink HydrolibBusApplication)

target_include_directories(hydrosp PUBLIC include)

target_compile_features(hydrosp PUBLIC cxx_std_20)
target_compile_options(hydrosp PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
    -Woverloaded-virtual -Wsuggest-override -fconcepts-diagnostics-depth=10)

set(LOGGER_TYPE "hydrolib")

include(FetchContent)

FetchContent_Declare(
    Hydrolib
    GIT_REPOSITORY https://github.com/SeaJackal/Hydrolib-soft.git
    GIT_TAG 84a4e358edeb0364f81e58edef76b341d2d23d55
)
    
FetchContent_MakeAvailable(Hydrolib)
