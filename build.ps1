#!/usr/bin/env pwsh
# Build script for SAGE Engine using NMake

param(
    [string]$Config = "Release",
    [switch]$Clean,
    [switch]$Configure
)

$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
$batFile = "$vsPath\VC\Auxiliary\Build\vcvars64.bat"
$buildDir = Split-Path -Parent $MyInvocation.MyCommand.Path | Join-Path -ChildPath "build"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "SAGE Engine Build Script" -ForegroundColor Cyan
Write-Host "Config: $Config" -ForegroundColor Cyan
Write-Host "BuildDir: $buildDir" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    if (Test-Path $buildDir) {
        Remove-Item -Path $buildDir -Recurse -Force
    }
    mkdir $buildDir -Force | Out-Null
    $Configure = $true
}

if ($Configure -or -not (Test-Path "$buildDir\Makefile")) {
    Write-Host "Configuring CMake..." -ForegroundColor Yellow
    $configCmd = @"
`"$batFile`" && cd /d `"$buildDir`" && cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=$Config -DCMAKE_CXX_COMPILER=cl.exe -DCMAKE_C_COMPILER=cl.exe
"@
    cmd /c $configCmd
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake configuration failed!" -ForegroundColor Red
        exit 1
    }
}

Write-Host "Building project..." -ForegroundColor Yellow
$buildCmd = @"
`"$batFile`" && cd /d `"$buildDir`" && nmake
"@
cmd /c $buildCmd
if ($LASTEXITCODE -eq 0) {
    Write-Host "========================================" -ForegroundColor Green
    Write-Host "Build completed successfully!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Green
} else {
    Write-Host "========================================" -ForegroundColor Red
    Write-Host "Build failed!" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Red
    exit 1
}
