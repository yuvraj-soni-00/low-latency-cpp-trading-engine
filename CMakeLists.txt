cmake_minimum_required(VERSION 3.21)

project(low_latency_trading_engine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")

# Gtest setup
include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

# Reusable library setup
add_library(trading_core STATIC
    src/order_book.cpp
    src/matching_engine.cpp
)
target_include_directories(trading_core
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

# Main application setup
add_executable(trading_engine
    src/main.cpp
)

target_link_libraries(trading_engine
    PRIVATE trading_core
)

# Testing setup
enable_testing()

# Executable test files
add_executable(order_book_tests
    test/order_book_tests.cpp
)
# Linking executable test files to library
target_link_libraries(order_book_tests
    PRIVATE
        trading_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(order_book_tests)

# Error functions for every compilation target
function(enable_project_warnings target_name)

    if(MSVC)
        target_compile_options(
            ${target_name}
            PRIVATE
            /W4
        )
    else()
        target_compile_options(
            ${target_name}
            PRIVATE
            -Wall
            -Wextra
            -Wpedantic
        )
    endif()

endfunction()

# Calling error function for each target
enable_project_warnings(trading_core)
enable_project_warnings(trading_engine)
enable_project_warnings(order_book_tests)
