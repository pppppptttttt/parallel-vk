cmake_minimum_required(VERSION 4.1)
project(hw1)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp)

target_compile_options(${PROJECT_NAME} PUBLIC -O0 -mavx2 -save-temps)
