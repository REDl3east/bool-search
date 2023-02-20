cmake_minimum_required(VERSION 3.0.0)
project(ez-search VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

add_compile_options(-g -O0)

add_executable(ez-search main.cpp)