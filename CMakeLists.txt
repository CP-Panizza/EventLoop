cmake_minimum_required(VERSION 3.11)
project(EventLoop)

set(CMAKE_CXX_STANDARD 11)

add_executable(EventLoop main.cpp EventLoop.h Event.hpp)