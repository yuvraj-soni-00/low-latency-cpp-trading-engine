cmake_minimum_required(VERSION 3.21)

project(low_latency_trading_engine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(order_book STATIC
    src/order_book.cpp
)
target_include_directories(order_book
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

add_executable(trading_engine
    src/main.cpp
)

target_link_libraries(trading_engine
    PRIVATE order_book
)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")


target_compile_options(trading_engine PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

add_executable(order_book_tests
    test/order_book_tests.cpp
    src/order_book.cpp
)

target_include_directories(order_book_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)