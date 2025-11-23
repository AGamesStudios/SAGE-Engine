@echo off
REM SAGE Engine - Quick Build (Windows CMD)
REM Быстрая компиляция игр одной командой

setlocal enabledelayedexpansion

set GAME_NAME=%1
set CONFIG=%2

if "%GAME_NAME%"=="" set GAME_NAME=TestGame
if "%CONFIG%"=="" set CONFIG=Release

echo ================================
echo    SAGE Engine - Quick Build
echo ================================
echo.

REM Проверка CMake
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found! Install CMake.
    exit /b 1
)

REM Шаг 1: Конфигурация
if not exist "build" (
    echo [1/3] Configuring project...
    cmake -S . -B build -G "Visual Studio 17 2022"
    if !errorlevel! neq 0 (
        echo [ERROR] CMake configuration failed!
        exit /b 1
    )
) else (
    echo [1/3] Project already configured [OK]
)

REM Шаг 2: Сборка движка
echo [2/3] Building engine...
cmake --build build --config %CONFIG% --target SAGE_Engine
if !errorlevel! neq 0 (
    echo [ERROR] Engine build failed!
    exit /b 1
)
echo        Engine built [OK]

REM Шаг 3: Сборка игры
echo [3/3] Building game '%GAME_NAME%'...
cmake --build build --config %CONFIG% --target %GAME_NAME%
if !errorlevel! neq 0 (
    echo [ERROR] Game build failed!
    exit /b 1
)
echo        Game ready [OK]

echo.
echo ================================
echo    SUCCESS! Game built!
echo ================================
echo.
echo Run: .\build\bin\%CONFIG%\%GAME_NAME%.exe
echo.

REM Запуск игры
set /p RUN="Run game now? (y/n): "
if /i "%RUN%"=="y" (
    echo.
    echo Starting game...
    .\build\bin\%CONFIG%\%GAME_NAME%.exe
)

endlocal
