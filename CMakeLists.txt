cmake_minimum_required(VERSION 3.10)

project(WebServer)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_BUILD_TYPE Debug)

include_directories(include)

file(GLOB SRC_LIST "src/UDP_Server.cpp")

add_executable(WebServer ${SRC_LIST})