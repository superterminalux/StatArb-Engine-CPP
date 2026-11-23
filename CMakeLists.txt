cmake_minimum_required(VERSION 3.15)
project(StatArb_Engine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        "C:/libs/eigen-5.0.1"
)

add_executable(StatArb_Engine
        src/main.cpp
        src/DataLoader.cpp
        src/OLSSolver.cpp
        src/SignalEngine.cpp
)