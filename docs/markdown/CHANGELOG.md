# Changelog

All notable changes to SAGE Engine will be documented in this file.

SAGE Engine is in early alpha. Version numbers not yet following semantic versioning.

---

## [Early Alpha] - 2025-01-07

First public release of SAGE Engine. This is experimental software.

### What Works
- Application lifecycle (OnInit, OnUpdate, OnRender, OnShutdown)
- Window creation with GLFW
- 2D batch rendering with OpenGL 3.3+
- Keyboard and mouse input
- Basic collision detection (AABB)
- Texture loading and rendering
- Sound playback (multi-channel)
- Logging system with printf formatting
- Math utilities (Vector2, Rect, Transform)
- CMake build system
- Standalone EXE builds (Windows)

### Known Issues
- Logger UTF-8 console output has encoding issues on some systems
- Renderer Init() may be called twice (warning shown)
- Collision detection fails with fast-moving objects
- No 3D rendering support
- No visual editor
- Documentation incomplete
- Test coverage partial
- Linux/macOS builds not thoroughly tested

### Limitations
- 2D only (no 3D)
- No ECS architecture yet
- No sprite animation system
- No tilemap support
- No scene serialization
- No networking
- No scripting integration
- Small community

### Examples Included
- SimpleGame - Basic engine usage
- PongTest - Full game example testing core features

### Build System
- CMake 3.20+ required
- MSVC 2019+, GCC 9+, Clang 10+ supported
- SAGE_STATIC_BUILD option for standalone executables
- Cross-platform (Windows tested, Linux/macOS experimental)

### Dependencies
- GLFW 3.3.8 (windowing)
- GLAD (OpenGL loader)
- stb_image (image loading)
- miniaudio (audio playback)

### License
MIT License - Free for commercial and personal use

---

## Unreleased (In Development)

Future improvements planned:
- Stabilize core systems
- Fix critical bugs
- Improve documentation
- Expand test coverage
- Add sprite animation
- Implement tilemap system
- Scene serialization
- Better error messages

---

For detailed version history, see git commits.
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
For detailed version history, see git commits.
