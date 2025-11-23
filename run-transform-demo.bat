@echo off
echo ===================================
echo   SAGE Engine - Transform Demo
echo ===================================
echo.
echo Building TransformDemo...
cmake --build build --config Release --target TransformDemo
if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)
echo.
echo Running TransformDemo...
echo.
.\build\bin\Release\TransformDemo.exe
pause
