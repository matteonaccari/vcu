cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(transmitter-simulator-avc CXX DESCRIPTION "Utility to simulate the transmission of H.264/AVC bistreams through noisy channels")


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(core)

add_executable(transmitter-simulator-avc main.cpp)

target_link_libraries(transmitter-simulator-avc PUBLIC core)

target_include_directories(transmitter-simulator-avc PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/core"
                          )