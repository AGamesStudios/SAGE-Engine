# 🚀 SAGE Engine - Installation Guide

Complete guide for installing and setting up SAGE Engine in your projects.

---

## 📋 Table of Contents

- [Quick Install](#-quick-install-recommended)
- [Manual Installation](#-manual-installation)
- [Using SAGE in Your Project](#-using-sage-in-your-project)
- [Verification](#-verification)
- [Troubleshooting](#-troubleshooting)
- [Platform-Specific Notes](#-platform-specific-notes)

---

## ⚡ Quick Install (Recommended)

### Windows

```batch
# Clone repository
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine

# Run installation script
scripts\install.bat
```

**Installation time:** ~2-3 minutes ⏱️

### Linux/macOS

```bash
# Clone repository
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine

# Make script executable and run
chmod +x scripts/install.sh
./scripts/install.sh
```

**Installation time:** ~2-3 minutes ⏱️

**✅ Done!** SAGE Engine is installed to `./install/`

---

## 🛠️ Manual Installation

### Prerequisites

| Requirement | Minimum Version | Recommended |
|------------|----------------|-------------|
| **CMake** | 3.20+ | 3.27+ |
| **C++ Compiler** | C++20 support | Latest |
| **Git** | Any | Latest |

**Supported Compilers:**
- **Windows:** MSVC 2019 16.11+ (Visual Studio 2019/2022)
- **Linux:** GCC 10+ or Clang 12+
- **macOS:** Clang 12+ (Xcode 13+)

### Step-by-Step Installation

#### 1️⃣ Clone Repository

```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
```

#### 2️⃣ Create Build Directory

```bash
mkdir build
cd build
```

#### 3️⃣ Configure CMake

**Windows (Visual Studio):**
```batch
cmake .. -G "Visual Studio 17 2022" -DCMAKE_INSTALL_PREFIX=../install
```

**Linux/macOS:**
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE=Release
```

**CMake Options:**

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory |
| `SAGE_BUILD_TESTS` | `ON` | Build unit tests |
| `SAGE_BUILD_EXAMPLES` | `ON` | Build example projects |
| `CMAKE_BUILD_TYPE` | `Debug` | Build type (Debug/Release) |

**Example with custom options:**
```bash
cmake .. \
  -DCMAKE_INSTALL_PREFIX=/opt/SAGE \
  -DCMAKE_BUILD_TYPE=Release \
  -DSAGE_BUILD_TESTS=OFF \
  -DSAGE_BUILD_EXAMPLES=ON
```

#### 4️⃣ Build

```bash
# Single-threaded build
cmake --build . --config Release

# Parallel build (faster)
cmake --build . --config Release --parallel
```

**Build times (approx):**
- Single-threaded: 5-10 minutes
- Parallel (8 cores): 1-2 minutes

#### 5️⃣ Run Tests (Optional)

```bash
ctest --output-on-failure
```

Expected output:
```
Test project SAGE-Engine/build
    Start 1: SAGE_Tests
1/1 Test #1: SAGE_Tests .......................   Passed    0.15 sec

100% tests passed, 0 tests failed out of 1
```

#### 6️⃣ Install

```bash
cmake --install . --config Release
```

**Installed files:**
```
install/
├── include/
│   ├── SAGE.h
│   └── SAGE/
│       ├── Audio/
│       ├── Core/
│       ├── Graphics/
│       ├── Input/
│       ├── Math/
│       ├── Physics/
│       ├── Resources/
│       ├── UI/
│       └── sage2d/
├── lib/
│   ├── cmake/SAGE/
│   │   ├── SAGEConfig.cmake
│   │   ├── SAGEConfigVersion.cmake
│   │   └── SAGETargets.cmake
│   ├── SAGE_Engine.lib       (Windows)
│   └── libSAGE_Engine.a      (Linux/macOS)
└── bin/                       (Examples)
```

---

## 🎯 Using SAGE in Your Project

### Method 1: CMake `find_package()` (Recommended)

**Project Structure:**
```
MyGame/
├── CMakeLists.txt
├── main.cpp
└── Assets/
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SAGE Engine
set(CMAKE_PREFIX_PATH "/path/to/SAGE/install")
find_package(SAGE REQUIRED)

# Create executable
add_executable(MyGame main.cpp)

# Link SAGE Engine
target_link_libraries(MyGame PRIVATE SAGE::SAGE_Engine)
```

**main.cpp:**
```cpp
#include <SAGE.h>

class MyGame : public SAGE::Application {
public:
    MyGame() : Application(800, 600, "My Game") {}
    
    void OnInit() override {
        SAGE_INFO("Game initialized!");
    }
    
    void OnUpdate(float deltaTime) override {
        // Game logic
    }
    
    void OnRender() override {
        SAGE::Renderer::BeginScene();
        // Rendering code
        SAGE::Renderer::EndScene();
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

**Build:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="/path/to/SAGE/install"
cmake --build .
./MyGame  # or MyGame.exe on Windows
```

### Method 2: Add as Subdirectory

**Project Structure:**
```
MyGame/
├── CMakeLists.txt
├── main.cpp
└── external/
    └── SAGE-Engine/  (git submodule or clone)
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyGame)

set(CMAKE_CXX_STANDARD 20)

# Add SAGE as subdirectory
add_subdirectory(external/SAGE-Engine/Engine)

# Create executable
add_executable(MyGame main.cpp)

# Link SAGE Engine
target_link_libraries(MyGame PRIVATE SAGE_Engine)
```

**Using Git Submodule:**
```bash
git submodule add https://github.com/AGamesStudios/SAGE-Engine.git external/SAGE-Engine
git submodule update --init --recursive
```

### Method 3: FetchContent (CMake 3.24+)

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.24)
project(MyGame)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
    SAGE
    GIT_REPOSITORY https://github.com/AGamesStudios/SAGE-Engine.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(SAGE)

add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE SAGE_Engine)
```

---

## ✅ Verification

### Verify Installation

Create a test project to verify SAGE is properly installed:

**test_sage.cpp:**
```cpp
#include <SAGE.h>
#include <iostream>

int main() {
    std::cout << "SAGE Engine Version: 0.1.0" << std::endl;
    std::cout << "Installation successful!" << std::endl;
    return 0;
}
```

**CMakeLists.txt:**
```cmake
cmake_minimum_required(VERSION 3.20)
project(TestSAGE)

set(CMAKE_PREFIX_PATH "/path/to/SAGE/install")
find_package(SAGE REQUIRED)

add_executable(TestSAGE test_sage.cpp)
target_link_libraries(TestSAGE PRIVATE SAGE::SAGE_Engine)
```

**Build and run:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="/path/to/SAGE/install"
cmake --build .
./TestSAGE
```

**Expected output:**
```
SAGE Engine Version: 0.1.0
Installation successful!
```

### CMake Verification

```bash
cmake .. -DCMAKE_PREFIX_PATH="/path/to/SAGE/install"
```

**Expected output:**
```
-- SAGE Engine found (version 0.1.0)
-- SAGE Engine include dir: /path/to/SAGE/install/include
-- Configuring done
-- Generating done
```

---

## 🔧 Troubleshooting

### ❌ "CMake Error: Could not find SAGE"

**Problem:** CMake cannot locate SAGE installation.

**Solution 1:** Set `CMAKE_PREFIX_PATH`:
```bash
cmake .. -DCMAKE_PREFIX_PATH="/path/to/SAGE/install"
```

**Solution 2:** Set environment variable:
```bash
# Windows (PowerShell)
$env:CMAKE_PREFIX_PATH = "C:\path\to\SAGE\install"

# Linux/macOS
export CMAKE_PREFIX_PATH="/path/to/SAGE/install"
```

**Solution 3:** Add to CMakeLists.txt:
```cmake
list(APPEND CMAKE_PREFIX_PATH "/path/to/SAGE/install")
find_package(SAGE REQUIRED)
```

### ❌ "C++20 features not available"

**Problem:** Compiler doesn't support C++20.

**Solution:** Update your compiler:

**Windows:**
- Install Visual Studio 2019 16.11+ or Visual Studio 2022
- Download: https://visualstudio.microsoft.com/

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install gcc-11 g++-11
```

**macOS:**
```bash
xcode-select --install  # Xcode Command Line Tools
# Or install full Xcode from App Store
```

**Verify compiler version:**
```bash
# Windows (MSVC)
cl /?

# Linux/macOS (GCC)
g++ --version

# macOS (Clang)
clang++ --version
```

### ❌ "GLFW/OpenGL not found"

**Problem:** Missing graphics dependencies.

**Solution:** SAGE bundles GLFW and GLAD, so this shouldn't happen. If it does:

**Windows:**
- Install Visual Studio with "Desktop development with C++" workload
- Install Windows SDK

**Linux:**
```bash
sudo apt install libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

**macOS:**
```bash
# OpenGL is included with macOS, no action needed
```

### ❌ Build Errors on Windows

**Problem:** LNK2019 or similar linker errors.

**Solution:**
1. Clean and rebuild:
   ```batch
   cmake --build . --config Release --clean-first
   ```

2. Ensure matching configuration:
   ```cmake
   # In your project
   cmake --build . --config Release  # Not Debug!
   ```

3. Check runtime library settings:
   ```cmake
   # Add to your CMakeLists.txt
   if(MSVC)
       set_property(TARGET MyGame PROPERTY 
           MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
   endif()
   ```

### ❌ "Permission Denied" on Linux/macOS

**Problem:** Cannot execute install script.

**Solution:**
```bash
chmod +x scripts/install.sh
./scripts/install.sh
```

---

## 🖥️ Platform-Specific Notes

### Windows

**Visual Studio Generator:**
```batch
cmake .. -G "Visual Studio 17 2022" -A x64
```

**Build Types:**
- Use `--config Release` or `--config Debug` with `cmake --build`
- Default is Debug

**Installation Path:**
- Avoid spaces in path: `C:\SAGE` instead of `C:\Program Files\SAGE`
- Use forward slashes in CMake: `-DCMAKE_INSTALL_PREFIX=C:/SAGE`

### Linux

**System-Wide Installation:**
```bash
sudo cmake --install . --prefix /usr/local
```

**User Installation:**
```bash
cmake --install . --prefix ~/.local
```

**ldconfig (if needed):**
```bash
sudo ldconfig
```

### macOS

**Xcode Generator:**
```bash
cmake .. -G Xcode
open SAGE_Engine.xcodeproj
```

**Universal Binary (Apple Silicon + Intel):**
```bash
cmake .. -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

**Framework Bundle (Advanced):**
```bash
cmake .. -DBUILD_FRAMEWORK=ON
```

---

## 📚 Next Steps

1. **Read Documentation:**
   - [QUICKSTART.md](QUICKSTART.md) - API overview and tutorials
   - [EXAMPLES.md](EXAMPLES.md) - Code examples and patterns
   - [README.md](README.md) - Project overview

2. **Explore Examples:**
   ```bash
   cd Examples/SimpleGame
   mkdir build && cd build
   cmake .. -DCMAKE_PREFIX_PATH="/path/to/SAGE/install"
   cmake --build .
   ```

3. **Join Community:**
   - GitHub Discussions: [SAGE-Engine/discussions](https://github.com/AGamesStudios/SAGE-Engine/discussions)
   - Report Issues: [SAGE-Engine/issues](https://github.com/AGamesStudios/SAGE-Engine/issues)

---

## 📞 Support

Need help? Choose your preferred channel:

- 💬 **Discord:** [Join our server](https://discord.gg/sage-engine)
- 🐛 **Bug Reports:** [GitHub Issues](https://github.com/AGamesStudios/SAGE-Engine/issues)
- 📧 **Email:** support@sage-engine.com
- 📖 **Documentation:** [Read the Docs](https://sage-engine.readthedocs.io)

---

**Happy Game Development! 🎮**

*Made with ❤️ by the SAGE Team*
