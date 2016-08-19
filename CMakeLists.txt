cmake_minimum_required(VERSION 2.8)
project(qplot)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(.)

set(qplot_src "main.cpp")

# Executable
add_executable(qplot ${qplot_src})
