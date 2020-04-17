cmake_minimum_required(VERSION 3.11)
project(EventLoop)

set(CMAKE_CXX_STANDARD 11)
IF (CMAKE_SYSTEM_NAME MATCHES "Windows")
    link_libraries("D:/MinGW64/x86_64-w64-mingw32/lib/libws2_32.a")
ELSE ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
ENDIF ()
add_executable(EventLoop
        main.cpp
        EventLoop.h
        Event.hpp
        CusEvent.hpp
        )