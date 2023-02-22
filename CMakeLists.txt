cmake_minimum_required(VERSION 3.0.0)
project(ez-search VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-g -O0)

set(MAIN_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

add_executable(ez-search ${MAIN_SRC})
target_include_directories(ez-search PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_target_properties(ez-search PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(TEST_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_eval.cpp
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

enable_testing()

FetchContent_MakeAvailable(googletest)

add_executable(test-eval ${TEST_SRC})
target_include_directories(test-eval PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test-eval GTest::gtest_main)
set_target_properties(test-eval PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")

include(GoogleTest)
gtest_discover_tests(test-eval)