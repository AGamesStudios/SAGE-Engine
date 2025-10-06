# 🎮 SAGE Engine

<div align="center">

**S**imple **A**dvanced **G**ame **E**ngine

*Powerful and Simple 2D Game Engine in C++ ## 📋 System Requirements

> **⚠️ Note**: These are preliminary requirements. Testing on various platforms ongoing.

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **OS** | Windows 10 / Linux / macOS 10.14 | Windows 11 / Latest Linux / macOS 12+ |
| **Compiler** | MSVC 2019 / GCC 9+ / Clang 10+ | MSVC 2022 / GCC 11+ / Clang 14+ |penGL*

![Version](https://img.shields.io/badge/version-Early%20Alpha-orange.svg)
![Status](https://img.shields.io/badge/status-In%20Development-yellow.svg)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](docs/markdown/LICENSE)
[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![OpenGL](https://img.shields.io/badge/OpenGL-3.3+-green.svg)](https://www.opengl.org/)
[![CMake](https://img.shields.io/badge/CMake-3.20+-blue.svg)](https://cmake.org/)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)

> ⚠️ **EARLY ALPHA WARNING**: This engine is in early alpha development stage. Expect bugs, missing features, and breaking changes. Please report any issues you encounter via [GitHub Issues](https://github.com/AGamesStudios/SAGE-Engine/issues).

[Quick Start](#-quick-start) • [Documentation](#-documentation) • [Examples](#-examples) • [Features](#-features)

</div>

---

## 📋 About

SAGE Engine is a modern 2D game engine built with C++ and OpenGL, currently in **early alpha development**. Designed for both learning game development and creating small to medium-sized 2D projects, SAGE provides a clean API and modular architecture.

> **⚠️ Alpha Status**: This project is under active development. APIs may change, features are incomplete, and you may encounter bugs. We welcome feedback and bug reports!

### ✨ Key Features (In Development)

> **Note**: Features marked with 🚧 are in early development and may be incomplete or unstable.

- 🎨 **2D Graphics** - OpenGL 3.3+ rendering pipeline 🚧
- 📦 **GameObject System** - Flexible GameObject architecture 🚧
- ⚙️ **Physics Engine** - Collision detection, AABB/Circle colliders 🚧
- 🎵 **Audio System** - Basic sound playback 🚧
- 🎭 **Particle Systems** - Customizable visual effects 🚧
- 📁 **Asset Management** - Resource loading and caching 🚧
- ⌨️ **Input System** - Keyboard, mouse, gamepad support 🚧
- 🎬 **Scene Management** - Scene and stage system 🚧
- 🖼️ **UI Framework** - Basic widgets (buttons, labels, panels) 🚧
- 📐 **Math Library** - Vector2, Rect, Transform utilities ✅
- 📝 **Logging System** - Multi-level logging ✅
- 🧪 **Test Suite** - Growing test coverage 🚧
- 🔧 **CMake Integration** - Cross-platform build system ✅

## 🚀 Quick Start

> **⚠️ Alpha Warning**: Installation process is functional but may have rough edges. Report issues on GitHub!

### Installation (5-10 minutes)

**Option 1: Automated Installation (Recommended)**

```powershell
# Windows
git clone https://github.com/yourusername/SAGE-Engine.git
cd SAGE-Engine
.\tools\install.bat
```

```bash
# Linux/macOS
git clone https://github.com/yourusername/SAGE-Engine.git
cd SAGE-Engine
chmod +x tools/install.sh
./tools/install.sh
```

**Option 2: Manual Installation**

See [INSTALL.md](INSTALL.md) for detailed instructions.

### Create Your First Game

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(MyGame)

set(CMAKE_CXX_STANDARD 20)
find_package(SAGE REQUIRED)

add_executable(MyGame main.cpp)
target_link_libraries(MyGame PRIVATE SAGE::SAGE_Engine)
```

```cpp
// main.cpp
#include <SAGE.h>

class MyGame : public Application {
public:
    MyGame() : Application("My First Game", 800, 600) {}
    
    void OnInitialize() override {
        // Create player
        GameObject* player = CreateGameObject("Player");
        player->SetPosition({400, 300});
        player->SetScale({50, 50});
        
        // Initialize systems
        SystemManager::GetAssetManager().LoadTexture("player", "assets/player.png");
        SystemManager::GetPhysicsSystem().SetGravity({0, -980});
    }
    
    void OnUpdate(float deltaTime) override {
        // Input handling
        if (Input::IsKeyPressed(KeyCode::ESCAPE)) {
            Shutdown();
        }
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

Build and run:
```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
.\build\Release\MyGame.exe
```

📖 **More examples:** See [Examples/](Examples/) directory

---

## � System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **OS** | Windows 10 / Linux / macOS 10.14 | Windows 11 / Latest Linux / macOS 12+ |
| **Compiler** | MSVC 2019 / GCC 9+ / Clang 10+ | MSVC 2022 / GCC 11+ / Clang 14+ |
| **CMake** | 3.20 | 3.25+ |
| **OpenGL** | 3.3 | 4.6 |
| **RAM** | 2 GB | 4 GB+ |

---

## � Documentation

```cpp
#include <SAGE.h>

class MyGame : public SAGE::Application {
public:
    MyGame() : Application("My Game") {}
    
    void OnInit() override {
        SAGE::Renderer::Init();
    }
    
    void OnRender() override {
        SAGE::Renderer::Clear(0.2f, 0.3f, 0.8f);
        SAGE::Renderer::BeginScene();
        
        // Рисуем красный квадрат через QuadDesc
        SAGE::Renderer::SetMaterial(SAGE::MaterialLibrary::GetDefaultId());

        SAGE::QuadDesc quad;
        quad.position = { 100.0f, 100.0f };
        quad.size = { 200.0f, 200.0f };
        quad.color = SAGE::Color::Red();
        SAGE::Renderer::DrawQuad(quad);
        
        SAGE::Renderer::EndScene();
    }
    
    void OnShutdown() override {
        SAGE::Renderer::Shutdown();
    }
};

SAGE::Application* SAGE::CreateApplication() {
    return new MyGame();
}

int main() {
    auto app = SAGE::CreateApplication();
    app->Run();
    delete app;
    return 0;
}
```

📖 **Больше примеров:** См. [EXAMPLES.md](EXAMPLES.md)

---

## � Examples

Check out the [Examples](Examples/) directory for complete working examples:

- **SimpleGame** - Basic game structure and setup
- More examples coming soon!

For detailed code samples, see [docs/guides/EXAMPLES.md](docs/guides/EXAMPLES.md).

---

## 🏗️ Project Structure

> **📦 Structure**: Core engine modularized for easy navigation and contribution

```
SAGE-Engine/
├── 📂 Engine/              # Core engine source code
│   ├── Core/              # Application, Window, Logger, Profiler
│   ├── Graphics/          # Renderer, Shader, Texture, Sprite, Particles
│   ├── Physics/           # Collision, Raycast, Forces
│   ├── Audio/             # Audio System
│   ├── Input/             # Input handling
│   ├── Resources/         # Asset management
│   ├── UI/                # UI widgets
│   ├── Math/              # Math utilities
│   ├── Memory/            # Memory management
│   ├── sage2d/            # 2D specific features
│   └── SAGE.h             # Main header
│
├── 📂 ThirdParty/         # External libraries
│   ├── glfw/              # GLFW 3.3.8
│   ├── glad/              # GLAD OpenGL loader
│   ├── stb/               # STB libraries
│   └── miniaudio/         # Audio library
│
├── 📂 Examples/           # Example projects
│   └── SimpleGame/        # Basic game example
│
├── 📂 Tests/              # Unit tests (30+ tests)
│   ├── TestFramework.cpp
│   └── ...
│
├── 📂 docs/               # Documentation
│   ├── guides/            # User guides
│   │   ├── GETTING_STARTED.md
│   │   ├── QUICKSTART.md
│   │   ├── SETUP.md
│   │   ├── INSTALL.md
│   │   └── EXAMPLES.md
│   └── markdown/          # Project documentation
│       ├── CHANGELOG.md
│       ├── CONTRIBUTING.md
│       ├── LICENSE
│       └── STRUCTURE.md
│
├── 📂 tools/              # Build and utility scripts
│   ├── install.bat        # Windows installer
│   └── install.sh         # Linux/macOS installer
│
├── 📂 Assets/             # Shared assets
├── 📂 .github/            # GitHub workflows
├── 📂 .vscode/            # VS Code settings
│
├── 📄 CMakeLists.txt      # Main CMake configuration
└── 📄 README.md           # This file
```

---

## 🗺️ Roadmap

> **🚧 Alpha Phase**: Currently focused on stabilizing core features before adding new systems

See [docs/markdown/STRUCTURE.md](docs/markdown/STRUCTURE.md) for detailed architecture.

### 🏗️ Alpha Stage (Current Focus)
- Stabilizing core systems (Application, Window, Logger, Profiler)
- Debugging 2D Graphics (Renderer, Shader, Texture, Sprite, Particles)
- Testing Physics (Collision, Raycast, Forces)
- Refining Audio (Multi-channel, Streaming)
- Improving Input (Keyboard, Mouse, Gamepad, Bindings)
- Optimizing Resources (Asset Manager, Async loading)
- Polishing UI (Button, Label, Panel)
- Validating Memory (Vault allocator)
- Verifying Math (Vector2, Rect, Transform)
- Expanding test coverage (30+ unit tests → more coverage)

### 📋 Beta Goals (Future)
- Tilemap system
- Sprite animation
- Scene serialization (JSON/Binary)
- Audio mixing and effects
- Improved particle effects
- Scene editor tools

### 🎯 v1.0 Release Goals (Long-term)
- Full ECS architecture
- Advanced physics (joints, constraints)
- Networking support
- Scripting integration (Lua/Python)
- Cross-platform mobile support

---

## ⚙️ CMake Options

- `SAGE_BUILD_TESTS` (default `ON`) — Build unit tests
- `SAGE_BUILD_EXAMPLES` (default `ON`) — Build example projects
- `SAGE_ENABLE_SANITIZERS` — Enable AddressSanitizer for Clang/GCC

---

## 🤝 Contributing

> **⚠️ Alpha Contributions**: Engine is unstable, expect breaking changes. Coordinate with maintainers before large PRs!

We welcome contributions! Please read our [Contributing Guidelines](docs/markdown/CONTRIBUTING.md) before submitting pull requests.

### Development Setup
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

**Note**: Please include tests for new features and ensure all existing tests pass.

---

## 📄 License

This project is licensed under the **MIT License** - see the [docs/markdown/LICENSE](docs/markdown/LICENSE) file for details.

```
MIT License
Copyright (c) 2025 A Games Studios
```

---

## 🙏 Acknowledgments

- [GLFW](https://www.glfw.org/) - Window and input handling
- [GLAD](https://glad.dav1d.de/) - OpenGL loader
- [stb_image](https://github.com/nothings/stb) - Image loading
- [miniaudio](https://miniaud.io/) - Audio playback

---

## 📞 Contact & Support

- 🐛 **Bug Reports:** [GitHub Issues](https://github.com/AGamesStudios/SAGE-Engine/issues)
- 💬 **Discussions:** [GitHub Discussions](https://github.com/AGamesStudios/SAGE-Engine/discussions)
- 🌐 **Repository:** [AGamesStudios/SAGE-Engine](https://github.com/AGamesStudios/SAGE-Engine)

---

<div align="center">

**⚠️ SAGE Engine is in early alpha - expect bugs, breaking changes, and incomplete features!**

Made with ❤️ by A Games Studios

</div>

<div align="center">

**Made with ❤️ by A Games Studios**

⭐ Star this repository if you find it helpful!

</div>
