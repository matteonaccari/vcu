cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(transmitter-simulator-avc CXX DESCRIPTION "Utility to simulate the transmission of H.264/AVC bistreams through noisy channels")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)

add_subdirectory(core)
add_subdirectory(unit-tests)

add_executable(transmitter-simulator-avc main.cpp)

set_target_properties(transmitter-simulator-avc PROPERTIES OUTPUT_NAME_DEBUG transmitter-simulator-avc-dbg)
set_target_properties(transmitter-simulator-avc PROPERTIES OUTPUT_NAME_RELEASE transmitter-simulator-avc)

target_link_libraries(transmitter-simulator-avc PUBLIC core)

target_include_directories(transmitter-simulator-avc PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/core"
                          )