cmake_minimum_required(VERSION 3.17)
project(CourseWork_MP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fopenmp")

add_executable(CourseWork_MP main.cpp)
