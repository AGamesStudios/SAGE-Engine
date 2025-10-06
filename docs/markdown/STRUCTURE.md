# 📁 SAGE Engine - Project Structure# SAGE Engine - Project Structure



This document provides a detailed overview of the SAGE Engine project structure.This document provides a detailed overview of the SAGE Engine project structure.



---## 📁 Root Directory Layout



## 🏗️ Root Directory Layout```

SAGE-Engine/

```├── 📂 Engine/              # Core engine source code

SAGE-Engine/├── 📂 Examples/            # Example projects

├── 📂 Engine/              # Core engine source code├── 📂 Tests/               # Unit tests

│   ├── Core/               # Application, Window, Logger, Scene, GameObject├── 📂 ThirdParty/          # External dependencies

│   ├── Graphics/           # Renderer, Shader, Texture, Sprite, Material├── 📂 Assets/              # Shared assets

│   ├── Physics/            # Collision, Collider, Raycast, Forces├── 📂 docs/                # Documentation

│   ├── Audio/              # Audio system and sound playback├── 📂 scripts/             # Build and utility scripts

│   ├── Input/              # Input polling, keycodes, bindings├── 📂 .github/             # GitHub workflows and configurations

│   ├── Resources/          # Asset management and loading├── 📂 .vscode/             # VS Code workspace settings

│   ├── UI/                 # UI system, widgets, buttons, panels├── 📂 build/               # Build output (gitignored)

│   ├── Math/               # Vector2, Rect, Transform, math utilities├── 📄 CMakeLists.txt       # Main CMake configuration

│   ├── Memory/             # Vault allocator├── 📄 CHANGELOG.md         # Version history

│   ├── sage2d/             # 2D-specific features├── 📄 CONTRIBUTING.md      # Contribution guidelines

│   └── cmake/              # CMake configuration modules├── 📄 LICENSE              # MIT License

│├── 📄 README.md            # Main project documentation

├── 📂 Examples/            # Example projects and demos├── 📄 STRUCTURE.md         # This file

│   ├── SimpleGame/         # Basic game example└── 📄 .gitignore           # Git ignore patterns

│   └── Demo/               # Feature demonstration```

│

├── 📂 Tests/               # Unit tests and integration tests---

│

├── 📂 ThirdParty/          # External dependencies (GLFW, GLAD, etc.)## 🔧 Engine Directory (`Engine/`)

│

├── 📂 Assets/              # Shared game assetsThe core engine source code organized by system:

│

├── 📂 docs/                # Documentation```

│   ├── markdown/           # Markdown documentationEngine/

│   │   ├── CHANGELOG.md    # Version history├── 📂 Core/                # Core systems

│   │   ├── CONTRIBUTING.md # Contribution guidelines│   ├── Application.h/cpp   # Main application class

│   │   ├── LICENSE         # MIT License│   ├── Window.h/cpp        # Window management

│   │   └── STRUCTURE.md    # This file│   ├── Logger.h/cpp        # Logging system

│   ├── api/                # API documentation│   ├── Profiler.h/cpp      # Performance profiler

│   ├── guides/             # User guides and tutorials│   ├── GameObject.h/cpp    # Game object base class

│   └── project/            # Project-specific documentation│   ├── Scene.h/cpp         # Scene management

││   ├── Stage.h/cpp         # Stage system

├── 📂 tools/               # Build scripts and utilities│   └── SystemManager.h/cpp # System coordinator

│   ├── install.bat         # Windows installation script│

│   └── install.sh          # Linux/macOS installation script├── 📂 Graphics/            # Rendering system

││   ├── Renderer.h/cpp      # Main renderer

├── 📂 build/               # Build output (gitignored)│   ├── Shader.h/cpp        # Shader management

││   ├── Texture.h/cpp       # Texture handling

├── 📂 .github/             # GitHub configurations│   ├── Sprite.h/cpp        # Sprite rendering

├── 📂 .vscode/             # VS Code workspace settings│   ├── Font.h/cpp          # Font rendering

││   ├── Material.h/cpp      # Material system

├── 📄 CMakeLists.txt       # Main CMake configuration│   └── ParticleSystem.h/cpp # Particle effects

├── 📄 README.md            # Main project documentation│

└── 📄 .gitignore           # Git ignore patterns├── 📂 Physics/             # Physics engine

```│   ├── Collision.h/cpp     # Collision detection

│   ├── Collider.h/cpp      # Collider shapes (AABB, Circle)

---│   ├── Raycast.h/cpp       # Raycasting

│   ├── PhysicsSystem.h/cpp # Physics coordinator

## 🔧 Engine Directory Structure│   └── Forces.h/cpp        # Gravity, friction

│

### Core System (`Engine/Core/`)├── 📂 Audio/               # Audio system

- Application.h/cpp - Main application class and game loop│   ├── AudioSystem.h/cpp   # Audio manager

- Window.h/cpp - Window creation and management│   └── Sound.h/cpp         # Sound playback

- Logger.h/cpp - Multi-level logging system│

- GameObject.h/cpp - Base game object class├── 📂 Input/               # Input handling

- Scene.h/cpp - Scene management│   ├── Input.h/cpp         # Input polling

- Stage.h/cpp - Stage system for game states│   ├── KeyCodes.h          # Keyboard key definitions

- SystemManager.h/cpp - Centralized system coordinator│   ├── MouseCodes.h        # Mouse button definitions

│   ├── GamepadCodes.h      # Gamepad button definitions

### Graphics System (`Engine/Graphics/`)│   └── InputBinding.h/cpp  # Input mapping system

- Renderer.h/cpp - Main rendering pipeline│

- Shader.h/cpp - Shader compilation and management├── 📂 Resources/           # Asset management

- Texture.h/cpp - Texture loading and handling│   ├── AssetManager.h/cpp  # Resource loading and caching

- Sprite.h/cpp - 2D sprite rendering│   ├── ResourceLoader.h/cpp # Async loading

- Material.h/cpp - Material system│   └── ResourceTypes.h     # Asset type definitions

- ParticleSystem.h/cpp - Particle effects│

├── 📂 UI/                  # User interface

### Physics System (`Engine/Physics/`)│   ├── UISystem.h/cpp      # UI manager

- Collision.h/cpp - Collision detection│   ├── Widget.h/cpp        # Base widget class

- Collider.h/cpp - Collider shapes (AABB, Circle)│   ├── Button.h/cpp        # Button widget

- Raycast.h/cpp - Raycasting│   ├── Label.h/cpp         # Text label

- PhysicsSystem.h/cpp - Physics coordinator│   └── Panel.h/cpp         # Container panel

- Forces.h/cpp - Gravity, friction│

├── 📂 Math/                # Mathematics

### Audio, Input, Resources, UI, Math, Memory│   ├── Vector2.h           # 2D vector

- See full documentation in respective directories│   ├── Rect.h              # Rectangle

│   ├── Transform.h         # Transform utilities

---│   └── Math.h              # Common math functions

│

## 📝 Key Files├── 📂 Memory/              # Memory management

│   └── Vault.h/cpp         # Custom allocator

### Root Level│

- **CMakeLists.txt** - Main build configuration├── 📂 sage2d/              # 2D-specific features

- **README.md** - Project overview and quick start│   └── [2D-specific code]

│

### Documentation (docs/markdown/)├── 📂 cmake/               # CMake modules

- **CHANGELOG.md** - Version history│   └── SAGEConfig.cmake.in # Package configuration

- **CONTRIBUTING.md** - How to contribute│

- **LICENSE** - MIT License├── 📄 CMakeLists.txt       # Engine CMake configuration

- **STRUCTURE.md** - This file└── 📄 SAGE.h               # Main engine header (includes all systems)

```

### Tools (tools/)

- **install.bat** - Windows installation---

- **install.sh** - Linux/macOS installation

## 🎮 Examples Directory (`Examples/`)

---

Sample projects demonstrating engine features:

**Last Updated**: October 6, 2025  

**Version**: 2.0.0 (rewrite/sage2d branch)```

Examples/
├── 📂 SimpleGame/          # Basic game example
│   ├── main.cpp           # Entry point
│   ├── CMakeLists.txt     # Build configuration
│   └── assets/            # Game assets
│
└── [More examples coming soon]
```

---

## 🧪 Tests Directory (`Tests/`)

Comprehensive test suite with 30+ unit tests:

```
Tests/
├── 📄 main.cpp                    # Test entry point
├── 📄 TestFramework.h/cpp         # Custom test framework
├── 📄 CoreSystemsTests.cpp        # Application, Window, Logger tests
├── 📄 GameObjectTests.cpp         # GameObject and Scene tests
├── 📄 InputBindingsTests.cpp      # Input system tests
├── 📄 MathTests.cpp               # Math utilities tests
├── 📄 ParticleSystemTests.cpp     # Particle system tests
├── 📄 RendererSmokeTests.cpp      # Renderer smoke tests
├── 📄 RendererTestShim.cpp        # Renderer test helpers
├── 📄 ResourceTests.cpp           # Asset manager tests
├── 📄 StageTests.cpp              # Stage system tests
├── 📄 UITests.cpp                 # UI widget tests
├── 📄 VaultTests.cpp              # Memory allocator tests
├── 📄 CMakeLists.txt              # Test build configuration
└── 📂 Data/                       # Test assets
```

---

## 📦 ThirdParty Directory (`ThirdParty/`)

External dependencies and libraries:

```
ThirdParty/
├── 📂 glfw/                # GLFW 3.3.8 (Window and input)
├── 📂 glad/                # GLAD (OpenGL loader)
├── 📂 stb/                 # STB libraries (image loading)
├── 📂 miniaudio/           # miniaudio (audio playback)
└── 📄 CMakeLists.txt       # Third-party build config
```

---

## 📚 Documentation Directory (`docs/`)

All project documentation:

```
docs/
├── 📂 guides/              # User guides
│   ├── GETTING_STARTED.md # Complete tutorial
│   ├── QUICKSTART.md      # Fast setup
│   ├── SETUP.md           # Environment setup
│   ├── INSTALL.md         # Installation guide
│   └── EXAMPLES.md        # Code samples
│
├── 📂 project/             # Project information
│   ├── ROADMAP.md         # Development roadmap
│   └── PROJECT_STATUS.md  # Current status
│
└── 📄 README.md            # Documentation index
```

---

## 🛠️ Scripts Directory (`scripts/`)

Build and utility scripts:

```
scripts/
├── 📄 install.bat          # Windows automated installer (120+ lines)
└── 📄 install.sh           # Linux/macOS automated installer (110+ lines)
```

---

## 🌐 GitHub Directory (`.github/`)

GitHub-specific configurations:

```
.github/
├── 📂 workflows/           # CI/CD workflows
│   └── [workflow files]
└── [GitHub templates]
```

---

## 💻 VS Code Directory (`.vscode/`)

VS Code workspace settings:

```
.vscode/
├── 📄 settings.json        # Workspace settings
├── 📄 tasks.json           # Build tasks
└── 📄 launch.json          # Debug configurations
```

---

## 🗄️ Assets Directory (`Assets/`)

Shared engine assets:

```
Assets/
└── [Shared resources]
```

---

## 🏗️ Build Directory (`build/`)

**Note**: This directory is gitignored and generated during build.

```
build/
├── 📂 Engine/              # Engine build output
├── 📂 Examples/            # Example builds
├── 📂 Tests/               # Test builds
├── 📂 ThirdParty/          # Third-party builds
└── [CMake-generated files]
```

---

## 📋 Key Files Explained

### Root Level Files

- **`CMakeLists.txt`**: Main CMake build configuration
  - Defines project name, version, and C++ standard
  - Adds subdirectories (Engine, Tests, Examples, ThirdParty)
  - Configures build options

- **`CHANGELOG.md`**: Version history following Keep a Changelog format
  - Documents all changes between versions
  - Categorized: Added, Changed, Deprecated, Removed, Fixed, Security

- **`CONTRIBUTING.md`**: Contributor guidelines
  - Code style requirements
  - Pull request process
  - Development setup instructions

- **`LICENSE`**: MIT License
  - Full license text
  - Copyright information

- **`README.md`**: Main project documentation
  - Quick start guide
  - Features overview
  - Installation instructions
  - Links to detailed documentation

- **`STRUCTURE.md`**: This file
  - Detailed project structure explanation

- **`.gitignore`**: Git ignore patterns
  - Build artifacts (build/, bin/, lib/)
  - IDE files (.vs/, .vscode/, *.user)
  - Compiled files (*.o, *.exe, *.dll)
  - Temporary files

---

## 🔄 Build Output Structure

When you build the project, CMake generates:

```
build/
├── Engine/
│   ├── Debug/
│   │   └── SAGE_Engine.lib
│   └── Release/
│       └── SAGE_Engine.lib
│
├── Examples/
│   └── SimpleGame/
│       ├── Debug/
│       │   └── SimpleGame.exe
│       └── Release/
│           └── SimpleGame.exe
│
├── Tests/
│   ├── Debug/
│   │   └── SAGE_Tests.exe
│   └── Release/
│       └── SAGE_Tests.exe
│
└── ThirdParty/
    └── [Library builds]
```

---

## 📦 Installation Output Structure

After running install scripts, you get:

```
install/
├── include/
│   └── SAGE/
│       ├── SAGE.h
│       ├── Core/
│       ├── Graphics/
│       ├── Physics/
│       ├── Audio/
│       ├── Input/
│       ├── Resources/
│       ├── UI/
│       ├── Math/
│       └── Memory/
│
├── lib/
│   ├── SAGE_Engine.lib (Windows)
│   └── libSAGE_Engine.a (Linux/macOS)
│
├── bin/
│   └── [Runtime DLLs if applicable]
│
└── share/
    └── SAGE/
        └── cmake/
            └── SAGEConfig.cmake
```

---

## 🎯 Navigation Tips

### Finding Specific Functionality

| Feature | Location |
|---------|----------|
| Application setup | `Engine/Core/Application.h` |
| Rendering | `Engine/Graphics/Renderer.h` |
| Physics | `Engine/Physics/PhysicsSystem.h` |
| Audio | `Engine/Audio/AudioSystem.h` |
| Input | `Engine/Input/Input.h` |
| Assets | `Engine/Resources/AssetManager.h` |
| UI | `Engine/UI/UISystem.h` |
| Math | `Engine/Math/` |

### Finding Documentation

| Topic | Location |
|-------|----------|
| Getting started | `docs/guides/GETTING_STARTED.md` |
| Installation | `docs/guides/INSTALL.md` |
| Code examples | `docs/guides/EXAMPLES.md` |
| Future plans | `docs/project/ROADMAP.md` |
| Version history | `CHANGELOG.md` |

### Finding Examples

| Example | Location |
|---------|----------|
| Basic game | `Examples/SimpleGame/` |
| Test code | `Tests/` (various test files) |

---

## 🔍 File Naming Conventions

- **Headers**: `ClassName.h`
- **Implementation**: `ClassName.cpp`
- **Tests**: `SystemNameTests.cpp`
- **Documentation**: `DOCUMENT_NAME.md` (uppercase)
- **Scripts**: `script_name.sh` or `script_name.bat`

---

## 📊 Code Organization

### Namespaces

- Main namespace: `SAGE`
- No sub-namespaces (kept flat for simplicity)

### Class Organization

```cpp
// Header file structure
#pragma once

namespace SAGE {

class ClassName {
public:
    // Public interface
    
private:
    // Private implementation
};

} // namespace SAGE
```

---

<div align="center">

**This structure is designed for clarity, maintainability, and ease of use.**

For questions or suggestions, see [CONTRIBUTING.md](CONTRIBUTING.md)

</div>
