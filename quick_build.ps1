# Quick Build Script for SAGE Engine
# Usage: .\quick_build.ps1 [target_name]
# Example: .\quick_build.ps1 SAGE_Engine
# Example: .\quick_build.ps1 Box2DPhysicsDemo

param(
    [string]$Target = "SAGE_Engine",
    [string]$Config = "Release"
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "SAGE Engine Quick Build" -ForegroundColor Cyan
Write-Host "Target: $Target | Config: $Config" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Measure build time
$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()

# Build only the specified target (much faster!)
cmake --build build --config $Config --target $Target

$stopwatch.Stop()
$elapsed = $stopwatch.Elapsed

Write-Host "`n========================================" -ForegroundColor Green
Write-Host "Build completed in: $($elapsed.ToString('mm\:ss\.ff'))" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# Show errors if any
if ($LASTEXITCODE -ne 0) {
    Write-Host "`nBuild FAILED! Check errors above." -ForegroundColor Red
    exit 1
} else {
    Write-Host "`nBuild SUCCESS!" -ForegroundColor Green
    
    # If building an example, show the path
    if ($Target -match "Demo|Example") {
        $exePath = "build\bin\$Config\$Target.exe"
        if (Test-Path $exePath) {
            Write-Host "Executable: $exePath" -ForegroundColor Yellow
        }
    }
}
