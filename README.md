# SAGE Engine

Simple Advanced Game Engine

A modern 2D game engine built with C++20 and OpenGL 3.3+

Version: Early Alpha  
Status: In Development  
License: MIT  
Platform: Windows, Linux, macOS

EARLY ALPHA WARNING: This engine is in early alpha development. Expect bugs, missing features, and breaking changes. Report issues via GitHub.

---

## About

SAGE Engine is a 2D game engine for learning C++20 and rapid prototyping. Built with modern C++ and OpenGL, it provides a clean API and transparent codebase.

Alpha Status: APIs may change. Features are incomplete. Bugs exist. Feedback welcome.


## Key Features

In Development - Core systems are functional but incomplete:

- 2D Graphics - OpenGL 3.3+ batch rendering
- Physics Engine - AABB collision detection
- Audio System - Multi-channel sound playback
- Input System - Keyboard and mouse support
- Asset Management - Texture and sound loading
- Scene System - Basic scene management
- UI Framework - Simple button and label widgets
- Math Library - Vector2, Rect, Transform (stable)
- Logging System - Multi-level debug output (stable)
- CMake Build - Cross-platform compilation (stable)

Stable features marked. Others are experimental.

## Quick Start

Installation takes approximately 5-10 minutes.

Windows:
```
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
.\tools\install.bat
```

Linux/macOS:
```
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
chmod +x tools/install.sh
./tools/install.sh
```

Manual installation: See INSTALL.md


## Example Code

Basic game structure:

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
        
        SAGE::QuadDesc quad;
        quad.position = {100.0f, 100.0f};
        quad.size = {200.0f, 200.0f};
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

Build:
```
cmake -S . -B build
cmake --build build --config Release
```

More examples in Examples/ directory.

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| OS | Windows 10, Linux, macOS 10.14 | Windows 11, Latest Linux, macOS 12+ |
| Compiler | MSVC 2019, GCC 9+, Clang 10+ | MSVC 2022, GCC 11+, Clang 14+ |
| CMake | 3.20 | 3.25+ |
| OpenGL | 3.3 | 4.6 |
| RAM | 2 GB | 4 GB+ |


## Project Structure

```
SAGE-Engine/
├── Engine/              # Core engine code
│   ├── Core/           # Application, Window, Logger
│   ├── Graphics/       # Renderer, Shader, Texture
│   ├── Physics/        # Collision, Forces
│   ├── Audio/          # Audio System
│   ├── Input/          # Input handling
│   ├── Resources/      # Asset management
│   ├── UI/             # UI widgets
│   ├── Math/           # Math utilities
│   └── SAGE.h          # Main header
│
├── ThirdParty/         # External libraries
│   ├── glfw/           # GLFW 3.3.8
│   ├── glad/           # OpenGL loader
│   └── stb/            # STB libraries
│
├── Examples/           # Example projects
│   ├── SimpleGame/     # Basic example
│   └── PongTest/       # Pong game test
│
├── Tests/              # Unit tests
├── docs/               # Documentation
└── CMakeLists.txt      # Build configuration
```

## Development Roadmap

Alpha Stage (Current):
- Stabilize core systems
- Fix critical bugs
- Improve documentation
- Expand test coverage

Beta Goals (Future):
- Tilemap system
- Sprite animation
- Scene serialization
- Audio effects

v1.0 Goals (Long-term):
- Full ECS architecture
- Advanced physics
- Networking support
- Scripting integration


## CMake Build Options

- SAGE_BUILD_TESTS (default ON) - Build unit tests
- SAGE_BUILD_EXAMPLES (default ON) - Build examples
- SAGE_STATIC_BUILD (default ON) - Standalone executable
- SAGE_ENABLE_SANITIZERS - AddressSanitizer for Clang/GCC

## Contributing

Engine is in alpha - breaking changes expected. Read CONTRIBUTING.md before submitting pull requests.

Development workflow:
1. Fork repository
2. Create feature branch
3. Commit changes
4. Push to branch
5. Open pull request

Include tests for new features.

## License

MIT License - see LICENSE file for details.

Copyright (c) 2025 A Games Studios

## Dependencies

- GLFW - Window and input handling
- GLAD - OpenGL loader
- stb_image - Image loading
- miniaudio - Audio playback

## Contact

Bug Reports: GitHub Issues
Discussions: GitHub Discussions
Repository: github.com/AGamesStudios/SAGE-Engine

---

SAGE Engine is early alpha software. Expect bugs and breaking changes.

Made by A Games Studios
