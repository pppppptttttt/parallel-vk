cmake_minimum_required(VERSION 4.0)
project(hw4)

set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp
        naive_intset.cpp
        fine_intset.cpp
        optimistic_intset.cpp
        lazy_intset.cpp)
