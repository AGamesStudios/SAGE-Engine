# SAGE Engine

**Version:** Alpha (Early Development)  
**License:** MIT  
**Platform:** Windows, Linux, macOS  
**Language:** C++17

Modern 2D game engine with Entity Component System architecture, Box2D physics integration, and OpenGL rendering.

## ⚠️ Alpha Status Warning

This engine is in **active development** and not yet production-ready. Features may be incomplete, APIs may change, and some systems require additional configuration. Use for learning, prototyping, and experimentation.

**See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for current limitations and workarounds.**

## Features

- **ECS Architecture** - Flexible Entity Component System design
- **Box2D Physics** - Full 2D physics simulation (v3.x)
- **OpenGL Rendering** - Hardware-accelerated 2D graphics with batching
- **Audio System** - Spatial audio powered by miniaudio
- **Input Management** - Keyboard, mouse, and gamepad support
- **Resource Manager** - Efficient asset loading and caching
- **Event System** - Decoupled event-driven architecture
- **Serialization** - JSON-based scene save/load system

## Documentation

Comprehensive documentation is available in the `docs/` directory:

- [CLI Guide](docs/CLI_GUIDE.md) - SAGE CLI command-line tool
- [Architecture Overview](docs/ARCHITECTURE.md) - Engine design and core systems
- [API Reference](docs/API_REFERENCE.md) - Complete API documentation
- [User Guide](docs/USER_GUIDE.md) - Tutorials and examples
- [Build Guide](docs/BUILD_GUIDE.md) - Build instructions for all platforms
- [Component Reference](docs/COMPONENT_REFERENCE.md) - Component documentation
- [System Reference](docs/SYSTEM_REFERENCE.md) - System API reference
- [Math API](docs/MATH_API_REFERENCE.md) - Mathematical utilities
- [Performance Guide](docs/PERFORMANCE_GUIDE.md) - Optimization techniques

## Quick Start

### Requirements

**System Requirements:**
- Windows 10/11, Ubuntu 20.04+, or macOS 10.15+
- OpenGL 3.3 compatible GPU
- 2 GB RAM minimum
- 500 MB disk space

**Build Tools:**
- Python 3.6+ (for SAGE CLI)
- CMake 3.15 or higher
- C++17 compatible compiler:
  - Visual Studio 2022 (Windows)
  - GCC 9+ (Linux)
  - Clang 10+ (macOS)
- Git

### Installation with SAGE CLI (Recommended)

SAGE CLI simplifies installation, building, and project management:

**1. Install SAGE CLI:**

```bash
cd SAGE-Engine/tools
python install_cli.py
```

**2. Install SAGE Engine:**

```bash
sage install
```

**3. Verify installation:**

```bash
sage test
sage info
```

**4. Create your first project:**

```bash
sage create MyGame
cd ../SAGEProjects/MyGame
sage project build
sage project run
```

See [CLI Guide](docs/CLI_GUIDE.md) for complete documentation.

### Manual Installation

If you prefer manual installation without CLI:

**1. Clone the repository with submodules:**

```bash
git clone --recursive https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
```

**2. Configure and build:**

**Windows (Visual Studio):**
```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

**Linux/macOS:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

**3. Run tests:**

```bash
./build/bin/Release/SAGETests
```

**4. Run examples:**

```bash
./build/bin/Release/Box2DPhysicsDemo
```

## Examples

The engine includes 17 example projects (requires `SAGE_BUILD_EXAMPLES=ON`):

- **Box2DPhysicsDemo** - Physics simulation with Box2D v3.x
- **PhysicsSandbox** - Interactive physics playground
- **UISystemTest** - UI rendering and interaction
- **AdvancedPostProcessDemo** - Post-processing effects
- **LogConDemo** - Console logging system
- **MusicSystemExample** - Music playback features
- And 11 more in `Examples/` directory

**Note:** Examples are currently in Alpha state and may require additional configuration.

## Testing

SAGE Engine includes comprehensive test coverage with unit and integration tests:

- ECS core functionality tests
- Physics integration tests  
- Graphics and camera tests
- Audio system tests
- Input management tests
- Serialization tests

Run all tests:
```bash
./build/bin/Release/SAGETests
```

## Project Structure

```
SAGE-Engine/
├── Engine/          # Core engine source code
│   ├── ECS/         # Entity Component System
│   ├── Graphics/    # Rendering systems
│   ├── Physics/     # Physics integration
│   ├── Audio/       # Audio systems
│   └── ...
├── Examples/        # Example projects
├── Tests/           # Unit and integration tests
├── ThirdParty/      # External dependencies
├── docs/            # Documentation
└── CMakeLists.txt   # Build configuration
```

## Dependencies

All dependencies are included as git submodules:

- **GLFW** 3.3+ - Window and input management
- **GLAD** - OpenGL loader
- **Box2D** 3.0+ - Physics engine
- **miniaudio** 0.11.23+ - Audio playback
- **stb_image** - Image loading
- **nlohmann/json** - JSON parsing
- **Catch2** - Testing framework

## Contributing

Contributions are welcome! Please read the contribution guidelines before submitting pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Status

**Current Version:** Alpha  
**Test Coverage:** 70+ tests  
**Examples:** 16 working demos  
**Platform Support:** Windows, Linux, macOS

The engine is in active development and suitable for creating 2D games and prototypes.
🔴 **Build** - требуется sol2 для сборки

### Using Object Pools

```cpp
#include "ECS/MemoryPool.h"

ObjectPool<Bullet> bulletPool(100);

// Spawn
Bullet* bullet = bulletPool.Acquire();

// Destroy
bulletPool.Release(bullet);
```

## Documentation

- **[ECS_OPTIMIZATION_GUIDE.md](ECS_OPTIMIZATION_GUIDE.md)** - Performance optimizations
- **[LOW_END_OPTIMIZATION.md](LOW_END_OPTIMIZATION.md)** - Low-end device support
- **[SYSTEMS_CLEANUP_SUMMARY.md](SYSTEMS_CLEANUP_SUMMARY.md)** - Removed systems info
- **[MIGRATION_QUICK.md](MIGRATION_QUICK.md)** - Quick migration guide
- **[Engine/ECS/Systems/README.md](Engine/ECS/Systems/README.md)** - System creation guide

## Core Components

- `CoreSystem.h` - Template-based system base class
- `MemoryPool.h` - Object pooling and component allocation
- `ChunkedStorage.h` - Chunk-based entity storage
- `ComponentArray.h` - Sparse-set component storage
- `VectorMath.h` - Batch vector operations
- `PhysicsSystem2D.h` - Lightweight 2D physics
- `LightweightSystems.h` - Example systems

## License

See LICENSE file.
