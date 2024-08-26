cmake_minimum_required(VERSION 3.10)

project(WebServer VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(WebServer ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build)

# 链接必要的库
# target_link_libraries(WebServer <libraries>)

# # 启用测试
# enable_testing()

# # 添加测试子目录
# add_subdirectory(test)