$ErrorActionPreference = "Stop"

Write-Host "📦 Packaging SAGE Engine v0.1.0 Alpha..." -ForegroundColor Cyan

# Define paths
$RootDir = Get-Location
$BuildDir = "$RootDir/build"
$DistDir = "$RootDir/dist/SAGE-Engine-0.1.0-Alpha"
$ZipFile = "$RootDir/dist/SAGE-Engine-0.1.0-Alpha.zip"

# Clean previous build
if (Test-Path $BuildDir) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Path $BuildDir -Recurse -Force
}
if (Test-Path "$RootDir/dist") {
    Remove-Item -Path "$RootDir/dist" -Recurse -Force
}

# Configure CMake (Release)
Write-Host "Configuring CMake (Release)..." -ForegroundColor Cyan
cmake --preset msvc-release

# Build (Release)
Write-Host "Building Engine (Release)..." -ForegroundColor Cyan
cmake --build --preset msvc-release

# Create distribution structure
Write-Host "Creating distribution structure..." -ForegroundColor Cyan
New-Item -ItemType Directory -Path "$DistDir/bin" -Force | Out-Null
New-Item -ItemType Directory -Path "$DistDir/lib" -Force | Out-Null
New-Item -ItemType Directory -Path "$DistDir/include" -Force | Out-Null
New-Item -ItemType Directory -Path "$DistDir/assets" -Force | Out-Null

# Copy Headers
Write-Host "Copying headers..."
Copy-Item -Path "$RootDir/Engine/include/SAGE" -Destination "$DistDir/include" -Recurse

# Copy Libs
Write-Host "Copying libraries..."
Copy-Item -Path "$BuildDir/msvc/lib/Release/*.lib" -Destination "$DistDir/lib"

# Copy Binaries (DLLs and Executables)
Write-Host "Copying binaries..."
Copy-Item -Path "$BuildDir/msvc/bin/Release/*.dll" -Destination "$DistDir/bin"
Copy-Item -Path "$BuildDir/msvc/bin/Release/*.exe" -Destination "$DistDir/bin"

# Copy Assets
Write-Host "Copying assets..."
Copy-Item -Path "$RootDir/assets/*" -Destination "$DistDir/assets" -Recurse

# Copy Docs
Copy-Item -Path "$RootDir/README.md" -Destination "$DistDir"
Copy-Item -Path "$RootDir/LICENSE" -Destination "$DistDir"
Copy-Item -Path "$RootDir/RELEASE_NOTES.md" -Destination "$DistDir"

# Zip it up
Write-Host "Compressing to ZIP..." -ForegroundColor Cyan
Compress-Archive -Path "$DistDir/*" -DestinationPath $ZipFile

Write-Host "✅ Package created successfully at: $ZipFile" -ForegroundColor Green
