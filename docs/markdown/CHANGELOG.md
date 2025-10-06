# Changelog

All notable changes to SAGE Engine will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

> **⚠️ Alpha Status**: SAGE Engine is in early alpha development. Version numbers are not yet following semantic versioning.

## [Unreleased]

### In Progress (Alpha Development)
- Stabilizing core engine systems
- Bug fixes and performance improvements
- Expanding test coverage
- Improving documentation
- Refining API design (breaking changes expected)

### Recently Added
- Professional installation system with CMake package config
- Automated installation scripts for Windows, Linux, and macOS
- Comprehensive documentation (INSTALL.md, EXAMPLES.md, CONTRIBUTING.md)
- SimpleGame example demonstrating engine usage
- Project reorganization (cleaner structure)

## [Alpha] - Early Development (2025)

> **Note**: This is an early alpha release. Features are incomplete and subject to breaking changes.

### Core Systems (Unstable)
- **Application**: Lifecycle management - ⚠️ Basic functionality, needs refinement
- **Window**: GLFW integration - ✅ Stable
- **Event System**: EventBus - 🚧 In development
- **Logger**: Multi-level logging - ✅ Stable
- **Profiler**: Performance monitoring - 🚧 Basic implementation
- **GameObject**: Entity system - 🚧 In development
- **Scene/Stage**: Management - 🚧 Basic implementation

### Graphics System (In Development)
- **Renderer**: 2D batch rendering with OpenGL 3.3+ - 🚧 Core works, needs optimization
- **Shader**: Management and compilation - 🚧 Basic functionality
- **Texture**: Loading and management - 🚧 Works but needs improvement
- **Material**: Material system - 🚧 Basic implementation
- **Font**: Embedded font rendering - 🚧 Experimental
- **Sprite**: Sprite rendering - 🚧 In development
- **Particles**: Particle system - 🚧 Early prototype
- **Color**: Utilities - ✅ Stable

### Physics System (Experimental)
- **Collision**: AABB and Circle detection - 🚧 Basic implementation
- **Raycast**: Raycasting - 🚧 Experimental
- **Forces**: Gravity, velocity, friction - 🚧 In development
- **Static/Dynamic**: Object types - 🚧 Basic support

### Audio System (Basic)
- **Playback**: Multi-channel audio - 🚧 Basic implementation
- **Sound**: Loading and management - 🚧 Works but limited
- **Volume**: Control - 🚧 Basic support
- **Streaming**: Audio streaming - 🚧 Experimental

### Input System (Functional)
- **Keyboard**: Pressed, just pressed, released - ✅ Stable
- **Mouse**: Buttons, position, delta - ✅ Stable
- **Gamepad**: Buttons and axes support - 🚧 Basic support
- **Bindings**: Input actions and axes - 🚧 In development
- **Mappings**: Configurable inputs - 🚧 Early implementation

### Resource Management (In Development)
- **AssetManager**: Async loading - 🚧 Basic implementation
- **Textures**: Caching and streaming - 🚧 In development
- **Sounds**: Management - 🚧 Basic support
- **Fonts**: Font management - 🚧 Experimental
- **Registry**: Resource tracking - 🚧 Basic implementation
- **Memory Tracking**: Usage monitoring - 🚧 In development

### UI System (Experimental)
- **Button**: Components - 🚧 Basic functionality
- **Label**: Text components - 🚧 In development
- **Panel**: Containers - 🚧 Experimental
- **Events**: Callbacks - 🚧 Basic support
- **Layout**: Management - 🚧 Early prototype

### Memory Management (Basic)
- **Vault**: Custom allocator - 🚧 In development
- **Smart Pointers**: CreateRef, CreateScope - ✅ Functional
- **Pooling**: Resource pooling - 🚧 Experimental

### Math Library (Stable)
- **Vector2**: Comprehensive operations - ✅ Stable
- **Rect**: Structure - ✅ Stable
- **Transform**: Utilities - ✅ Stable
- **Collision Helpers**: Detection helpers - 🚧 In development

### Build System (Functional)
- **CMake**: 3.20+ configuration - ✅ Stable
- **Visual Studio**: 2022 support - ✅ Works
- **GCC/Clang**: Support - ✅ Works
- **Cross-platform**: Windows, Linux, macOS - 🚧 Tested on Windows, Linux/macOS needs testing
- **Package Config**: find_package() support - ✅ Functional
- **Installation**: Targets - ✅ Works

### Testing (Functional)
- **Framework**: Custom test framework - ✅ Works
- **Unit Tests**: 30+ tests - ✅ Most passing
- **Core Tests**: Systems tests - 🚧 Some failures expected
- **Physics Tests**: Physics validation - 🚧 In development
- **Renderer Tests**: Smoke tests - 🚧 Basic coverage
- **Input Tests**: Binding tests - 🚧 Partial coverage
- **Resource Tests**: Management tests - 🚧 Basic coverage
- **UI Tests**: System tests - 🚧 Limited coverage
- **Integration Tests**: System integration - 🚧 Early stage

### Documentation (In Progress)
- **README**: Quick start guide - ✅ Updated
- **INSTALL**: Installation instructions - ✅ Complete
- **EXAMPLES**: Code tutorials - 🚧 Basic examples
- **CONTRIBUTING**: Guidelines - ✅ Complete
- **QUICKSTART**: API reference - 🚧 Needs expansion
- **GETTING_STARTED**: Beginner guide - 🚧 Basic coverage
- **STRUCTURE**: Architecture docs - ✅ Complete

### Third-Party Libraries (Integrated)
- **GLFW**: 3.3+ windowing - ✅ Integrated
- **GLAD**: OpenGL loading - ✅ Integrated
- **stb_image**: Image loading - ✅ Integrated
- **stb_truetype**: Font rendering - 🚧 Experimental
- **miniaudio**: Audio playback - 🚧 Basic integration

---

## Known Issues (Alpha)

### Critical Bugs
- ⚠️ Renderer may have memory leaks in batch rendering
- ⚠️ Particle system crashes on certain configurations
- ⚠️ Physics collision detection has edge case failures
- ⚠️ Audio system may crash on rapid sound loading

### Missing Features
- 🚧 No scene serialization yet
- 🚧 No tilemap system
- 🚧 No sprite animation system
- 🚧 Limited UI widget set
- 🚧 No ECS architecture yet

### Platform Issues
- ⚠️ Linux/macOS builds not thoroughly tested
- ⚠️ Gamepad support incomplete on non-Windows platforms
- 🚧 OpenGL compatibility issues on some hardware

### API Instability
- ⚠️ Breaking changes expected in future updates
- 🚧 API design still being refined
- 🚧 Function signatures may change

---

## Alpha Development Notes

**Current Status**: This engine is in **early alpha**. It is **not production-ready**.

### What Works
- ✅ Basic window creation and event handling
- ✅ Simple 2D sprite rendering
- ✅ Basic keyboard and mouse input
- ✅ Math utilities (vectors, transforms)
- ✅ Logging system
- ✅ CMake build system

### What's Unstable
- 🚧 Advanced rendering features (particles, materials)
- 🚧 Physics system (collision detection unreliable)
- 🚧 Audio system (crashes possible)
- 🚧 Resource management (memory leaks possible)
- 🚧 UI system (very basic)
- 🚧 Gamepad input

### What's Missing
- ❌ Scene serialization
- ❌ Tilemap support
- ❌ Animation system
- ❌ Networking
- ❌ Scripting support
- ❌ Advanced editor tools

**Recommendation**: Use for learning, prototyping, and experimentation only. Not suitable for production games.

---

## Version History

### Alpha (2025-01-06)
- Initial alpha release
- Core systems implemented but unstable
- Breaking changes expected
- Active development ongoing
- LNK4098 library linking warning on Windows (non-critical)

**System Requirements:**
- C++20 compatible compiler
- CMake 3.20+
- OpenGL 3.3+
- 2GB RAM minimum, 4GB recommended

**Download:**
- Source code available on GitHub
- Pre-built binaries available via releases
- Installation via CMake package system

**Getting Started:**
See [INSTALL.md](INSTALL.md) for installation instructions and [EXAMPLES.md](EXAMPLES.md) for tutorials.

---

## Future Releases

### Planned for 0.2.0
- Tilemap system
- Animation system (sprite sheets)
- Scene serialization (save/load)
- Asset hot-reloading
- Advanced camera features
- 2D lighting system

See [ROADMAP.md](ROADMAP.md) for complete future plans.

---

[Unreleased]: https://github.com/AGamesStudios/SAGE-Engine/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/AGamesStudios/SAGE-Engine/releases/tag/v0.1.0
