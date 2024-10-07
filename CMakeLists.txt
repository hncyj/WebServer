cmake_minimum_required(VERSION 3.10)

project(WebServer VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(MYSQL_INCLUDE_DIRS "/usr/include/mysql")
set(MYSQL_LIBRARIES "-L/usr/lib/x86_64-linux-gnu -lmysqlclient -lz -lzstd -lssl -lcrypto -lresolv -lm")

include_directories(${MYSQL_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(fmt REQUIRED)

option(BUILD_TESTS_ONLY "Build tests only" OFF)

if(NOT BUILD_TESTS_ONLY)
    file(GLOB_RECURSE SOURCES 
        ${PROJECT_SOURCE_DIR}/src/main.cpp
        ${PROJECT_SOURCE_DIR}/src/*/*.cpp
    )
    add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
    target_link_libraries(${CMAKE_PROJECT_NAME} 
        PRIVATE 
        fmt::fmt 
        ${MYSQL_LIBRARIES}
    )
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE PROJECT_ROOT=\"${CMAKE_SOURCE_DIR}\")
endif()

message(STATUS "PROJECT_ROOT is: ${CMAKE_SOURCE_DIR}")

# 启用测试
enable_testing()

if(BUILD_TESTS_ONLY OR CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    add_subdirectory(test)
endif()