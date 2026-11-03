cmake_minimum_required(VERSION 3.21)

project(low_latency_trading_engine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(engine_core STATIC
    src/engine.cpp
)
target_include_directories(engine_core 
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include  
)

add_executable(trading_engine
    src/main.cpp
)

target_link_libraries(trading_engine
    PRIVATE engine_core
)

target_compile_options(trading_engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)