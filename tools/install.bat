@echo off
REM ====================================
REM SAGE Engine - Installation Script
REM ====================================

setlocal enabledelayedexpansion

echo.
echo ====================================
echo   SAGE Engine Installation
echo ====================================
echo.

REM Configuration
set "BUILD_DIR=build"
set "INSTALL_DIR=%CD%\install"

REM Colors (Windows 10+)
set "COLOR_GREEN=[92m"
set "COLOR_YELLOW=[93m"
set "COLOR_RED=[91m"
set "COLOR_RESET=[0m"

echo %COLOR_YELLOW%[CONFIG]%COLOR_RESET% Build Directory: %BUILD_DIR%
echo %COLOR_YELLOW%[CONFIG]%COLOR_RESET% Install Directory: %INSTALL_DIR%
echo.

REM Step 1: Create build directory
echo %COLOR_YELLOW%[1/5]%COLOR_RESET% Creating build directory...
if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
    echo %COLOR_GREEN%✓%COLOR_RESET% Created %BUILD_DIR%
) else (
    echo %COLOR_GREEN%✓%COLOR_RESET% Using existing %BUILD_DIR%
)
cd "%BUILD_DIR%"

REM Step 2: Configure CMake
echo.
echo %COLOR_YELLOW%[2/5]%COLOR_RESET% Configuring CMake...
cmake .. ^
    -G "Visual Studio 17 2022" ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DSAGE_BUILD_TESTS=OFF ^
    -DSAGE_BUILD_EXAMPLES=ON

if %ERRORLEVEL% NEQ 0 (
    echo %COLOR_RED%✗ ERROR:%COLOR_RESET% CMake configuration failed!
    cd ..
    pause
    exit /b 1
)
echo %COLOR_GREEN%✓%COLOR_RESET% CMake configured successfully

REM Step 3: Build Debug
echo.
echo %COLOR_YELLOW%[3/5]%COLOR_RESET% Building SAGE Engine (Debug)...
cmake --build . --config Debug --parallel

if %ERRORLEVEL% NEQ 0 (
    echo %COLOR_RED%✗ ERROR:%COLOR_RESET% Debug build failed!
    cd ..
    pause
    exit /b 1
)
echo %COLOR_GREEN%✓%COLOR_RESET% Debug build completed

REM Step 4: Build Release
echo.
echo %COLOR_YELLOW%[4/5]%COLOR_RESET% Building SAGE Engine (Release)...
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo %COLOR_RED%✗ ERROR:%COLOR_RESET% Release build failed!
    cd ..
    pause
    exit /b 1
)
echo %COLOR_GREEN%✓%COLOR_RESET% Release build completed

REM Step 5: Install
echo.
echo %COLOR_YELLOW%[5/5]%COLOR_RESET% Installing SAGE Engine...
cmake --install . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo %COLOR_RED%✗ ERROR:%COLOR_RESET% Installation failed!
    cd ..
    pause
    exit /b 1
)
echo %COLOR_GREEN%✓%COLOR_RESET% Installation completed

cd ..

REM Success message
echo.
echo ====================================
echo   Installation Successful! 🎉
echo ====================================
echo.
echo %COLOR_GREEN%SAGE Engine installed to:%COLOR_RESET%
echo   %INSTALL_DIR%
echo.
echo %COLOR_YELLOW%To use SAGE in your project:%COLOR_RESET%
echo.
echo   1. Create CMakeLists.txt:
echo      cmake_minimum_required(VERSION 3.20)
echo      project(MyGame)
echo.
echo      set(CMAKE_PREFIX_PATH "%INSTALL_DIR%")
echo      find_package(SAGE REQUIRED)
echo.
echo      add_executable(MyGame main.cpp)
echo      target_link_libraries(MyGame PRIVATE SAGE::SAGE_Engine)
echo.
echo   2. Create main.cpp with #include ^<SAGE.h^>
echo.
echo   3. Build your project:
echo      mkdir build ^&^& cd build
echo      cmake .. -DCMAKE_PREFIX_PATH="%INSTALL_DIR%"
echo      cmake --build .
echo.
echo %COLOR_YELLOW%Examples:%COLOR_RESET%
echo   Check Examples/ folder for sample projects
echo.
echo %COLOR_YELLOW%Documentation:%COLOR_RESET%
echo   - INSTALL.md - Installation guide
echo   - QUICKSTART.md - API reference
echo   - EXAMPLES.md - Code examples
echo.

pause
