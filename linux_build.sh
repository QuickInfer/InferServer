#!/bin/bash
echo "=== InferServer Build Script for Linux ==="

if [ -d "build" ]; then
    echo "Cleaning previous build directory..."
    rm -rf build
fi

mkdir -p build
cd build

cmake ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

cmake --build .
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
echo "Executable location: $(pwd)/bin/infer_server"
cd ..