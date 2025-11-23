param(
    [ValidateSet("all", "configure", "build", "rebuild", "clean", "test", "list", "ci", "help")]
    [string]$Action = "build",
    [string]$Preset = "dev-fast-ninja",
    [switch]$Tests,
    [switch]$Verbose,
    [switch]$ShowArtifacts
)

$ErrorActionPreference = "Stop"

Write-Host "[DEBUG] Action = '$Action'" -ForegroundColor Magenta

$projectRoot = (Resolve-Path (Join-Path $PSScriptRoot ".." )).Path
$presetsPath = Join-Path $projectRoot "CMakePresets.json"

# ═══════════════════════════════════════════════════════════════
# COLORS & PRETTY PRINTING
# ═══════════════════════════════════════════════════════════════

function Write-Header {
    param([string]$Text)
    Write-Host ""
    Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Blue
    Write-Host "  $Text" -ForegroundColor Cyan
    Write-Host "═══════════════════════════════════════════════════════════════" -ForegroundColor Blue
    Write-Host ""
}

function Write-Step {
    param([string]$Text)
    Write-Host "▶ $Text" -ForegroundColor Yellow
}

function Write-Success {
    param([string]$Text)
    Write-Host "✓ $Text" -ForegroundColor Green
}

function Write-Failure {
    param([string]$Text)
    Write-Host "✗ $Text" -ForegroundColor Red
}

function Write-Info {
    param([string]$Text)
    Write-Host "  $Text" -ForegroundColor Gray
}

Write-Host "[DEBUG] Functions defined" -ForegroundColor Magenta

# ═══════════════════════════════════════════════════════════════
# HELP COMMAND
# ═══════════════════════════════════════════════════════════════

Write-Host "[DEBUG] About to check if Action == help" -ForegroundColor Magenta

if ($Action -eq "help") {
    Write-Host "[DEBUG] Inside help block!" -ForegroundColor Magenta
    Write-Header "SAGE-Engine Build System"
    
    Write-Host "USAGE:" -ForegroundColor Cyan
    Write-Info "  .\scripts\sage-build.ps1 [OPTIONS]"
    Write-Host ""
    
    Write-Host "ACTIONS:" -ForegroundColor Cyan
    Write-Info '  list          Show all available CMake presets'
    Write-Info '  configure     Configure CMake (generate build files)'
    Write-Info '  build         Build the project (default)'
    Write-Info '  rebuild       Clean rebuild (--clean-first)'
    Write-Info '  clean         Remove build directory'
    Write-Info '  test          Run tests with ctest'
    Write-Info '  ci            Full CI pipeline (configure + build + test)'
    Write-Info '  help          Show this help message'
    Write-Host ""
    
    Write-Host "OPTIONS:" -ForegroundColor Cyan
    Write-Info "  -Preset NAME      CMake preset to use (default: dev-fast-ninja)"
    Write-Info "  -Tests            Run tests after build"
    Write-Info "  -Verbose          Show full command output"
    Write-Info "  -ShowArtifacts    Show build artifacts after build"
    Write-Host ""
    
    Write-Host "EXAMPLES:" -ForegroundColor Cyan
    Write-Info "  .\scripts\sage-build.ps1                              # Quick build with Ninja"
    Write-Info "  .\scripts\sage-build.ps1 -Action list                 # List all presets"
    Write-Info "  .\scripts\sage-build.ps1 -Preset dev-fast-msvc        # Build with Visual Studio"
    Write-Info "  .\scripts\sage-build.ps1 -Action rebuild -Tests       # Clean rebuild + run tests"
    Write-Info "  .\scripts\sage-build.ps1 -Preset release-speed-ninja  # Release build"
    Write-Host ""
    
    Write-Host "AVAILABLE PRESETS:" -ForegroundColor Cyan
    Write-Info "  dev-fast-ninja        Fast iterative builds (Ninja, PCH OFF)"
    Write-Info "  dev-minimal-ninja     Minimal features, fastest iteration"
    Write-Info "  dev-debug-ninja       Debug build with symbols (Ninja)"
    Write-Info "  release-speed-ninja   Optimized for performance (/O2)"
    Write-Info "  release-size-ninja    Optimized for size (/O1)"
    Write-Info "  dev-fast-msvc         Fast build (Visual Studio 2022)"
    Write-Info "  dev-debug-msvc        Debug build (Visual Studio 2022)"
    Write-Host ""
    
    exit 0
}

if (-not (Test-Path $presetsPath)) {
    Write-Error "CMakePresets.json not found. Run this script from inside the repository."
    exit 1
}

function Invoke-Tool {
    param(
        [string]$Executable,
        [string[]]$Arguments,
        [string]$WorkingDirectory = $projectRoot
    )

    if ($Verbose) {
        Write-Host ":: $Executable $($Arguments -join ' ')" -ForegroundColor Gray
    } else {
        Write-Host ":: $Executable $($Arguments -join ' ')" -ForegroundColor DarkGray
    }

    Push-Location $WorkingDirectory
    try {
        & $Executable @Arguments
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Resolve-BinaryDir {
    param(
        $PresetEntry,
        [string]$PresetName
    )

    if (-not $PresetEntry) {
        return $null
    }

    $path = $PresetEntry.binaryDir

    if (-not $path -and $PresetEntry.inherits) {
        $inheritList = @()
        if ($PresetEntry.inherits -is [System.Array]) {
            $inheritList = $PresetEntry.inherits
        } else {
            $inheritList = @($PresetEntry.inherits)
        }

        foreach ($parentName in $inheritList) {
            $parentEntry = $presetsJson.configurePresets | Where-Object { $_.name -eq $parentName }
            $path = Resolve-BinaryDir -PresetEntry $parentEntry -PresetName $PresetName
            if ($path) {
                break
            }
        }
    }

    if (-not $path) {
        return $null
    }

    $result = $path.Replace('${sourceDir}', $projectRoot)
    if ($PresetName) {
        $result = $result.Replace('${presetName}', $PresetName)
    }

    return [System.IO.Path]::GetFullPath($result)
}

$presetsJson = Get-Content $presetsPath -Raw | ConvertFrom-Json

if ($Action -eq "list") {
    Write-Header "SAGE-Engine Available Presets"
    
    Write-Host "CONFIGURE PRESETS:" -ForegroundColor Cyan
    Invoke-Tool -Executable "cmake" -Arguments @("--list-presets=configure")
    Write-Host "" 
    
    Write-Host "BUILD PRESETS:" -ForegroundColor Cyan
    Invoke-Tool -Executable "cmake" -Arguments @("--list-presets=build")
    Write-Host "" 
    
    Write-Host "TEST PRESETS:" -ForegroundColor Cyan
    Invoke-Tool -Executable "ctest" -Arguments @("--list-presets")
    Write-Host ""
    
    Write-Success "Use '.\scripts\sage-build.ps1 -Preset <name>' to build with a specific preset"
    Write-Info "Run '.\scripts\sage-build.ps1 -Action help' for more information"
    
    exit 0
}

$configurePreset = $presetsJson.configurePresets | Where-Object { $_.name -eq $Preset }
if (-not $configurePreset) {
    Write-Error "Configure preset '$Preset' not found. Use -Action list to inspect available presets."
    exit 1
}

$buildPreset = $presetsJson.buildPresets | Where-Object { $_.name -eq $Preset }
$testPreset = $presetsJson.testPresets | Where-Object { $_.name -eq $Preset }
$binaryDir = Resolve-BinaryDir -PresetEntry $configurePreset -PresetName $Preset

if ($configurePreset.generator -like "*Ninja*" -and -not (Get-Command ninja -ErrorAction SilentlyContinue)) {
    Write-Error "Preset '$Preset' requires Ninja, but Ninja was not found in PATH. Install Ninja or use an MSVC preset."
    exit 1
}

function Ensure-Configured {
    if (-not (Test-Path (Join-Path $binaryDir "CMakeCache.txt"))) {
        Write-Step "Configuring (preset: $Preset)"
        Invoke-Tool -Executable "cmake" -Arguments @("--preset", $Preset)
    }
}

function Show-BuildArtifacts {
    param([string]$BinaryDir)
    
    Write-Host ""
    Write-Host "BUILD ARTIFACTS:" -ForegroundColor Cyan
    
    $exeFiles = Get-ChildItem -Path $BinaryDir -Recurse -Filter "*.exe" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch "\\CMakeFiles\\" }
    $dllFiles = Get-ChildItem -Path $BinaryDir -Recurse -Filter "*.dll" -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch "\\CMakeFiles\\" }
    
    if ($exeFiles) {
        Write-Info "Executables:"
        foreach ($file in $exeFiles) {
            $sizeMiB = [math]::Round($file.Length / 1048576, 2)
            $relPath = $file.FullName.Replace($BinaryDir, "").TrimStart("\\")
            Write-Host "    * $relPath - $sizeMiB MiB" -ForegroundColor White
        }
    }
    
    if ($dllFiles) {
        Write-Info "Dynamic Libraries:"
        $count = 0
        foreach ($file in $dllFiles) {
            if ($count -ge 10) { break }
            $sizeMiB = [math]::Round($file.Length / 1048576, 2)
            $relPath = $file.FullName.Replace($BinaryDir, "").TrimStart("\\")
            Write-Host "    * $relPath - $sizeMiB MiB" -ForegroundColor White
            $count++
        }
        if ($dllFiles.Count -gt 10) {
            Write-Info "    ... and $($dllFiles.Count - 10) more DLLs"
        }
    }
    
    Write-Host ""
}

switch ($Action) {
    "clean" {
        if (Test-Path $binaryDir) {
            Write-Step "Cleaning build directory: $binaryDir"
            Remove-Item -Path $binaryDir -Recurse -Force
            Write-Success "Build directory cleaned"
        } else {
            Write-Info "Binary directory is already clean."
        }
    }
    "configure" {
        Write-Step "Configuring (preset: $Preset)"
        Invoke-Tool -Executable "cmake" -Arguments @("--preset", $Preset)
        Write-Success "Configuration complete"
    }
    "build" {
        Ensure-Configured
        if (-not $buildPreset) {
            Write-Failure "No build preset named '$Preset' was defined."
            exit 1
        }
        $sw = [System.Diagnostics.Stopwatch]::StartNew()
        Write-Step "Building (preset: $Preset)"
        Invoke-Tool -Executable "cmake" -Arguments @("--build", "--preset", $Preset)
        $sw.Stop()
        Write-Success "Build completed in $([math]::Round($sw.Elapsed.TotalSeconds, 2))s"
        
        if ($ShowArtifacts) {
            Show-BuildArtifacts -BinaryDir $binaryDir
        }
        
        if ($Tests -and $testPreset) {
            Write-Step "Running tests (preset: $Preset)"
            Invoke-Tool -Executable "ctest" -Arguments @("--preset", $Preset)
        } elseif ($Tests) {
            Write-Host "No test preset defined for '$Preset'. Skipping tests." -ForegroundColor Yellow
        }
    }
    "rebuild" {
        Ensure-Configured
        if (-not $buildPreset) {
            Write-Failure "No build preset named '$Preset' was defined."
            exit 1
        }
        $sw = [System.Diagnostics.Stopwatch]::StartNew()
        Write-Step "Rebuilding (clean first, preset: $Preset)"
        Invoke-Tool -Executable "cmake" -Arguments @("--build", "--preset", $Preset, "--clean-first")
        $sw.Stop()
        Write-Success "Rebuild completed in $([math]::Round($sw.Elapsed.TotalSeconds, 2))s"
        
        if ($ShowArtifacts) {
            Show-BuildArtifacts -BinaryDir $binaryDir
        }
        
        if ($Tests -and $testPreset) {
            Write-Step "Running tests (preset: $Preset)"
            Invoke-Tool -Executable "ctest" -Arguments @("--preset", $Preset)
        } elseif ($Tests) {
            Write-Host "No test preset defined for '$Preset'. Skipping tests." -ForegroundColor Yellow
        }
    }
    "test" {
        Ensure-Configured
        if (-not $testPreset) {
            Write-Failure "No test preset named '$Preset' was defined."
            exit 1
        }
        Write-Step "Running tests (preset: $Preset)"
        Invoke-Tool -Executable "ctest" -Arguments @("--preset", $Preset)
        Write-Success "Tests complete"
    }
    "ci" {
        Write-Header "CI Pipeline (preset: $Preset)"
        
        Write-Step "Step 1/3: Configure"
        Invoke-Tool -Executable "cmake" -Arguments @("--preset", $Preset)
        
        $sw = [System.Diagnostics.Stopwatch]::StartNew()
        Write-Step "Step 2/3: Build"
        Invoke-Tool -Executable "cmake" -Arguments @("--build", "--preset", $Preset)
        $sw.Stop()
        Write-Success "Build completed in $([math]::Round($sw.Elapsed.TotalSeconds, 2))s"
        
        if ($testPreset) {
            Write-Step "Step 3/3: Test"
            Invoke-Tool -Executable "ctest" -Arguments @("--preset", $Preset)
            Write-Success "CI pipeline complete"
        } else {
            Write-Host "No test preset defined for '$Preset'. Skipping tests." -ForegroundColor Yellow
            Write-Success "CI pipeline complete (without tests)"
        }
    }
    default {
        Ensure-Configured
        if (-not $buildPreset) {
            Write-Failure "No build preset named '$Preset' was defined."
            exit 1
        }
        $sw = [System.Diagnostics.Stopwatch]::StartNew()
        Write-Step "Configuring (preset: $Preset)"
        Invoke-Tool -Executable "cmake" -Arguments @("--preset", $Preset)
        Write-Step "Building (preset: $Preset)"
        Invoke-Tool -Executable "cmake" -Arguments @("--build", "--preset", $Preset)
        $sw.Stop()
        Write-Success "Build completed in $([math]::Round($sw.Elapsed.TotalSeconds, 2))s"
        
        if ($ShowArtifacts) {
            Show-BuildArtifacts -BinaryDir $binaryDir
        }
        
        if ($Tests -and $testPreset) {
            Write-Step "Running tests (preset: $Preset)"
            Invoke-Tool -Executable "ctest" -Arguments @("--preset", $Preset)
        } elseif ($Tests) {
            Write-Host "No test preset defined for '$Preset'. Skipping tests." -ForegroundColor Yellow
        }
    }
}
