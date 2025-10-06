# Standalone Build Guide

## Overview

SAGE Engine supports building **standalone executables** that don't require any external DLL files or Visual Studio redistributables to run. This makes it easy to distribute your games to players without complicated installation procedures.

## What is Static Linking?

By default, MSVC applications link to **dynamic runtime libraries** (DLLs like `MSVCR140.dll`, `VCRUNTIME140.dll`, `MSVCP140.dll`). This means:
- ❌ Players need Visual Studio redistributables installed
- ❌ Your game directory contains multiple DLL files
- ❌ Version conflicts can occur between different applications

**Static linking** embeds the runtime library directly into the EXE:
- ✅ Single `.exe` file with no dependencies
- ✅ Works on any Windows machine without installation
- ✅ No DLL conflicts
- ✅ Easier distribution (just ZIP and share!)

## How to Build Standalone EXE

### Step 1: Configure CMake with Static Build Option

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -DSAGE_STATIC_BUILD=ON
```

The `-DSAGE_STATIC_BUILD=ON` flag enables:
- **Static runtime linkage** (`/MT` for Release, `/MTd` for Debug)
- **No console window** in Release builds (WIN32_EXECUTABLE)
- **No external DLL dependencies**

### Step 2: Build in Release Mode

```powershell
cmake --build build --config Release -j 8
```

This produces standalone `.exe` files in `build/Examples/<ProjectName>/Release/`

### Step 3: Test the EXE

Just run it directly - no setup needed:

```powershell
.\build\Examples\PongTest\Release\PongTest.exe
```

If it works, you're done! The EXE is standalone.

## Size Comparison

| Build Type | Size | DLL Dependencies |
|------------|------|------------------|
| Dynamic (/MD) | ~200 KB | MSVCR140.dll (100 KB), VCRUNTIME140.dll (80 KB), MSVCP140.dll (600 KB) |
| **Static (/MT)** | **~1.2 MB** | **None!** |

While the static EXE is larger, you only distribute **one file** instead of multiple DLLs.

## Distributing Your Game

### Simple Distribution:
1. Build with `SAGE_STATIC_BUILD=ON` in Release mode
2. Copy `<YourGame>.exe` from `build/Examples/<YourGame>/Release/`
3. Create a ZIP with:
   - `YourGame.exe`
   - `README.txt` (controls, credits, etc.)
   - Any asset files (textures, sounds, etc.) your game loads

That's it! Players can extract and run `YourGame.exe` directly.

### Example:
```
MyAwesomeGame.zip
├── MyAwesomeGame.exe  (1.5 MB, standalone!)
├── README.txt
└── Assets/
    ├── textures/
    └── sounds/
```

## Pros and Cons

### Advantages ✅
- **Zero installation** - just run the EXE
- **No redistributables** required
- **Single file** for core application
- **No DLL version conflicts**
- **Portable** - works on any Windows PC

### Disadvantages ❌
- **Larger EXE** (~1 MB more vs. dynamic)
- **Longer link time** (negligible for small projects)
- **Static runtime** means entire CRT is in EXE

For **game distribution**, the advantages far outweigh the disadvantages!

## Troubleshooting

### "This app can't run on your PC" Error

This usually means architecture mismatch. Make sure:
- You're building for `x64` (default in SAGE)
- Target machine is 64-bit Windows

### Missing GLFW or OpenGL Errors

SAGE uses OpenGL 3.3+. Ensure:
- Graphics drivers are up to date
- OpenGL 3.3 support (most GPUs from 2010+)

### Game Runs Slow

Release builds are optimized. If still slow:
- Check GPU drivers
- Reduce window resolution
- Use VSync to cap framerate

## Advanced: CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `SAGE_STATIC_BUILD` | `ON` | Enable static runtime linkage |
| `CMAKE_BUILD_TYPE` | `Release` | Build configuration |
| `BUILD_SHARED_LIBS` | `OFF` | Build DLLs instead of static libs |

## Creating Your Own Project

When creating a new game project, add to your `CMakeLists.txt`:

```cmake
# Apply static build settings if enabled
if(SAGE_STATIC_BUILD AND WIN32 AND MSVC)
    # Hide console in Release builds
    set_target_properties(MyGame PROPERTIES
        WIN32_EXECUTABLE $<$<CONFIG:Release>:TRUE>
    )
    
    # Static runtime linkage
    target_compile_options(MyGame PRIVATE
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Debug>:/MTd>
    )
endif()
```

This ensures your game builds as standalone when `SAGE_STATIC_BUILD=ON`.

## Platform Notes

### Windows ✅
Fully supported with MSVC. Tested on Windows 10/11.

### Linux 🚧
Not yet tested. Static linking on Linux uses different flags (`-static-libgcc`, `-static-libstdc++`).

### macOS 🚧
Not yet tested. macOS uses framework bundles instead of raw EXEs.

---

**Happy distributing!** 🎮

For questions or issues, see [CONTRIBUTING.md](../../CONTRIBUTING.md) or open an issue on GitHub.
