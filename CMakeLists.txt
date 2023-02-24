cmake_minimum_required(VERSION 3.0.0)
project(bool-search VERSION 0.1.0)

option(BOOL_SEARCH_COMPILE_TESTS "Weather or not to compile the tests. Will install gtest." ON)

add_executable(bool-search
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/argtable3/argtable3.c
)
target_include_directories(bool-search PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/libs
  ${CMAKE_CURRENT_SOURCE_DIR}/libs/argtable3
)
set_property(TARGET bool-search PROPERTY CXX_STANDARD 17)
set_target_properties(bool-search PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(BOOL_SEARCH_COMPILE_TESTS)
  enable_testing()

  include(FetchContent)

  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  )

  FetchContent_MakeAvailable(googletest)

  add_executable(test-eval
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_eval.cpp
  )
  target_include_directories(test-eval PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )
  target_link_libraries(test-eval GTest::gtest_main)
  set_target_properties(test-eval PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")

  include(GoogleTest)
  gtest_discover_tests(test-eval)

  install(TARGETS bool-search test-eval DESTINATION bin)
else()
  install(TARGETS bool-search DESTINATION bin)
endif(BOOL_SEARCH_COMPILE_TESTS)


