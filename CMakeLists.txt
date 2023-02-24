cmake_minimum_required(VERSION 3.0.0)
project(bool-search VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-g -O0)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs)

set(MAIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(bool-search ${MAIN_SRC})
target_link_libraries(bool-search argtable3)
target_include_directories(bool-search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set_target_properties(bool-search PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_eval.cpp
)

add_executable(test-eval ${TEST_SRC})
target_include_directories(test-eval PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test-eval GTest::gtest_main)
set_target_properties(test-eval PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")

include(GoogleTest)
gtest_discover_tests(test-eval)