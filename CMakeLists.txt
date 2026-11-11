cmake_minimum_required(VERSION 3.10)
project(SafeBoard_2026p2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/code")
add_library(file_collector_lib ${SRC_DIR}/src/FileCollector.cpp)
target_include_directories(file_collector_lib PUBLIC ${SRC_DIR}/include)
target_link_libraries(file_collector_lib PUBLIC Threads::Threads)
include(FetchContent)

FetchContent_Declare(googletest URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

add_executable(FileCollector_tests ${SRC_DIR}/results/FileCollector_tests.cpp)
target_link_libraries(FileCollector_tests PRIVATE file_collector_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(FileCollector_tests)