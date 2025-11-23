# SAGE Engine - Quick Build Script
param(
    [string]$GameName = "TestGame",
    [string]$Config = "Release"
)

Write-Host "================================" -ForegroundColor Cyan
Write-Host "   SAGE Engine - Quick Build   " -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host ""

$ErrorActionPreference = "Stop"

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "[ERROR] CMake not found!" -ForegroundColor Red
    exit 1
}

try {
    if (-not (Test-Path "build")) {
        Write-Host "[1/3] Configuring project..." -ForegroundColor Yellow
        cmake -S . -B build -G "Visual Studio 17 2022"
        if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }
    } else {
        Write-Host "[1/3] Project configured [OK]" -ForegroundColor Green
    }

    Write-Host "[2/3] Building engine..." -ForegroundColor Yellow
    cmake --build build --config $Config --target SAGE_Engine
    if ($LASTEXITCODE -ne 0) { throw "Engine build failed" }
    Write-Host "       Engine built [OK]" -ForegroundColor Green

    Write-Host "[3/3] Building game '$GameName'..." -ForegroundColor Yellow
    cmake --build build --config $Config --target $GameName
    if ($LASTEXITCODE -ne 0) { throw "Game build failed" }
    Write-Host "       Game ready [OK]" -ForegroundColor Green

    Write-Host ""
    Write-Host "================================" -ForegroundColor Green
    Write-Host "   SUCCESS! Game built!         " -ForegroundColor Green
    Write-Host "================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Run: .\build\bin\$Config\$GameName.exe" -ForegroundColor Cyan
    Write-Host ""

    $run = Read-Host "Run game now? (y/n)"
    if ($run -eq 'y' -or $run -eq 'Y' -or $run -eq '') {
        Write-Host ""
        Write-Host "Starting game..." -ForegroundColor Cyan
        & ".\build\bin\$Config\$GameName.exe"
    }

} catch {
    Write-Host ""
    Write-Host "================================" -ForegroundColor Red
    Write-Host "   BUILD ERROR!                 " -ForegroundColor Red
    Write-Host "================================" -ForegroundColor Red
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}
