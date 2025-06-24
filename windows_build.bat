@echo off
echo === InferServer Build Script for Windows ===

if exist build (
    echo Cleaning previous build directory...
    rmdir /s /q build
)
if not exist build mkdir build
cd build

cmake -G "Visual Studio 17 2022" ..
if %ERRORLEVEL% NEQ 0 (
    echo CMake configuration failed!
    exit /b %ERRORLEVEL%
)

cmake --build .
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo Build completed successfully!
echo Executable location: %CD%\bin\infer_server.exe
cd ..