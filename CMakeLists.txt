cmake_minimum_required(VERSION 3.10)

project(WebServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

include_directories(include)

file(GLOB SRC_LIST "src/*.cpp")

add_executable(WebServer ${SRC_LIST})