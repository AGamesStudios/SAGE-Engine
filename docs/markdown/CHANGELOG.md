# Changelog

All notable changes to SAGE Engine will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Professional installation system with CMake package config
- Automated installation scripts for Windows, Linux, and macOS
- Comprehensive documentation (INSTALL.md, EXAMPLES.md, CONTRIBUTING.md)
- SimpleGame example demonstrating engine usage
- CI/CD pipeline with GitHub Actions

## [0.1.0] - 2025-10-06

### Added
- **Core Systems**
  - Application lifecycle management
  - Window management with GLFW
  - Event system with EventBus
  - Logger with multiple log levels
  - Profiler for performance monitoring
  - GameObject entity system
  - Scene and Stage management

- **Graphics System**
  - 2D batch renderer with OpenGL 3.3+
  - Shader management and compilation
  - Texture loading and management
  - Material system
  - Font rendering with embedded fonts
  - Sprite rendering
  - Particle system
  - Color utilities

- **Physics System**
  - AABB (Box) collision detection
  - Circle collision detection
  - Raycast functionality
  - Gravity simulation
  - Velocity and force application
  - Friction and bounciness
  - Static/dynamic objects

- **Audio System**
  - Multi-channel audio playback
  - Sound loading and management
  - Volume control
  - Audio streaming support

- **Input System**
  - Keyboard input (pressed, just pressed, released)
  - Mouse input (buttons, position, delta)
  - Gamepad support (buttons, axes)
  - Input binding system (actions and axes)
  - Configurable input mappings

- **Resource Management**
  - AssetManager with async loading
  - Texture caching and streaming
  - Sound management
  - Font management
  - Resource registry
  - Memory usage tracking

- **UI System**
  - Button components
  - Label components
  - Panel containers
  - Event callbacks
  - Layout management

- **Memory Management**
  - Vault allocator for efficient resource management
  - Smart pointer utilities (CreateRef, CreateScope)
  - Resource pooling

- **Math Library**
  - Vector2 with comprehensive operations
  - Rect structure
  - Transform utilities
  - Collision detection helpers

- **Build System**
  - CMake 3.20+ build configuration
  - Visual Studio 2022 support
  - GCC and Clang support
  - Cross-platform compatibility (Windows, Linux, macOS)
  - Package configuration for find_package()
  - Installation targets

- **Testing**
  - Custom test framework
  - 30+ comprehensive unit tests
  - Core systems tests
  - Physics tests
  - Renderer smoke tests
  - Input binding tests
  - Resource management tests
  - UI system tests
  - Integration tests

- **Documentation**
  - README with quick start guide
  - INSTALL.md with detailed installation instructions
  - EXAMPLES.md with code tutorials
  - CONTRIBUTING.md with contributor guidelines
  - QUICKSTART.md for API reference
  - GETTING_STARTED.md for beginners
  - ROADMAP.md for future plans
  - PROJECT_STATUS.md showing capabilities

- **Third-Party Libraries**
  - GLFW 3.3+ for windowing
  - GLAD for OpenGL loading
  - stb_image for image loading
  - stb_truetype for font rendering
  - miniaudio for audio playback

### Changed
- N/A (Initial release)

### Deprecated
- N/A (Initial release)

### Removed
- N/A (Initial release)

### Fixed
- N/A (Initial release)

### Security
- N/A (Initial release)

---

## Release Notes

### Version 0.1.0 - Initial Release

This is the first official release of SAGE Engine, a modern 2D game engine built with C++20 and OpenGL. The engine provides a comprehensive set of features for creating 2D games, including advanced graphics, physics simulation, audio playback, and more.

**Highlights:**
- ✅ Production-ready 2D rendering with batch optimization
- ✅ Complete physics system with collision detection
- ✅ Multi-channel audio with streaming support
- ✅ Flexible input system with customizable bindings
- ✅ Efficient resource management with async loading
- ✅ Comprehensive test coverage (30+ tests, 100% pass rate)
- ✅ Professional installation system with CMake
- ✅ Cross-platform support (Windows, Linux, macOS)
- ✅ Complete documentation and examples

**Known Issues:**
- Minor C4244 conversion warnings from STL (non-critical)
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
