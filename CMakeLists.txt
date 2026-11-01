cmake_minimum_required(VERSION 3.20)
project(StatArb_Engine_CPP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Warnings
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Eigen: add direct location (if find_package is exists)
include_directories("C:/libs/eigen-5.0.1")

add_executable(statarb main.cpp)