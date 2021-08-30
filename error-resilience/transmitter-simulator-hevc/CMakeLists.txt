cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(transmitter-simulator-hevc CXX DESCRIPTION "Utility to simulate the transmission of H.265/HEVC bistreams through noisy channels")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)

add_subdirectory(core)
add_subdirectory(unit-tests)

add_executable(transmitter-simulator-hevc main.cpp)

set_target_properties(transmitter-simulator-hevc PROPERTIES OUTPUT_NAME_DEBUG transmitter-simulator-hevc-dbg)
set_target_properties(transmitter-simulator-hevc PROPERTIES OUTPUT_NAME_RELEASE transmitter-simulator-hevc)

target_link_libraries(transmitter-simulator-hevc PUBLIC core)

target_include_directories(transmitter-simulator-hevc PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/core"
                          )