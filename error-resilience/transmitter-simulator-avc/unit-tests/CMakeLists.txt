add_executable(unit-tests test.cpp)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)
include_directories(${PROJECT_SOURCE_DIR}/core)
target_link_libraries(unit-tests PUBLIC core gtest pthread)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set_target_properties(unit-tests PROPERTIES OUTPUT_NAME_DEBUG unit-tests-dbg)
set_target_properties(unit-tests PROPERTIES OUTPUT_NAME_RELEASE unit-tests)