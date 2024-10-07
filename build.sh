#!/bin/bash

set -e

if [ -d "build" ]; then
    echo "Removing existing build directory..."
    rm -rf build
fi

if [ -d "logfiles" ]; then
    echo "Removing existing logfiles directory..."
    rm -rf logfiles
fi

# 创建新的 build 目录
mkdir build
cd build

# 执行 CMake 和 make
cmake ..
make