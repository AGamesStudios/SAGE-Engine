@echo off
REM Установка переменных окружения Visual Studio 2022
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Переход в директорию проекта
cd /d "c:\Users\amcki\Documents\AmckinatorProject\SAGE-Engine"

REM Сборка Release
echo.
echo Building SAGE-Engine Release...
echo.
cmake --build build --config Release

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build SUCCESS!
    echo.
    echo Running SAGE_Tests.exe...
    .\build\bin\Release\SAGE_Tests.exe
) else (
    echo.
    echo Build FAILED with error code %ERRORLEVEL%
    pause
    exit /b %ERRORLEVEL%
)
