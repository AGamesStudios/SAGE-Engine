# Contributing to SAGE Engine

Thank you for your interest in contributing to SAGE Engine! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Process](#development-process)
- [Coding Standards](#coding-standards)
- [Submitting Changes](#submitting-changes)
- [Testing Guidelines](#testing-guidelines)
- [Documentation](#documentation)

## Code of Conduct

By participating in this project, you agree to maintain a respectful and collaborative environment. We expect all contributors to:

- Use welcoming and inclusive language
- Be respectful of differing viewpoints and experiences
- Accept constructive criticism gracefully
- Focus on what is best for the community
- Show empathy towards other community members

## Getting Started

### Prerequisites

Before contributing, ensure you have:

- Git installed and configured
- CMake 3.15 or higher
- C++17 compatible compiler (Visual Studio 2022, GCC 9+, or Clang 10+)
- Basic understanding of ECS architecture
- Familiarity with Box2D physics (for physics-related contributions)

### Setting Up Development Environment

1. Fork the repository on GitHub
2. Clone your fork locally:

```bash
git clone --recursive https://github.com/YOUR_USERNAME/SAGE-Engine.git
cd SAGE-Engine
```

3. Add the upstream repository:

```bash
git remote add upstream https://github.com/AGamesStudios/SAGE-Engine.git
```

4. Create a development branch:

```bash
git checkout -b feature/your-feature-name
```

5. Build the project:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

6. Run tests to ensure everything works:

```bash
./build/bin/Debug/SAGETests
```

## Development Process

### Branching Strategy

- `main` - Stable release branch
- `develop` - Integration branch for features
- `feature/*` - New features
- `bugfix/*` - Bug fixes
- `hotfix/*` - Critical production fixes

### Workflow

1. Sync with upstream:

```bash
git fetch upstream
git checkout main
git merge upstream/main
```

2. Create a feature branch from `main`:

```bash
git checkout -b feature/amazing-feature
```

3. Make your changes following our coding standards

4. Write or update tests for your changes

5. Ensure all tests pass:

```bash
cmake --build build --config Debug
./build/bin/Debug/SAGETests
```

6. Update documentation if needed

7. Commit your changes with clear messages

8. Push to your fork:

```bash
git push origin feature/amazing-feature
```

9. Open a Pull Request on GitHub

## Coding Standards

### C++ Style Guide

**Naming Conventions:**

- **Classes/Structs:** PascalCase
  ```cpp
  class TransformComponent { };
  struct PhysicsData { };
  ```

- **Functions/Methods:** PascalCase
  ```cpp
  void UpdatePhysics(float deltaTime);
  Entity CreateEntity();
  ```

- **Variables:** camelCase
  ```cpp
  float deltaTime;
  Vector2 playerPosition;
  ```

- **Member Variables:** camelCase (no prefix)
  ```cpp
  class Player {
      float health;
      Vector2 position;
  };
  ```

- **Constants:** UPPER_SNAKE_CASE
  ```cpp
  constexpr int MAX_ENTITIES = 10000;
  const float PI = 3.14159f;
  ```

- **Enums:** PascalCase for enum name, UPPER_SNAKE_CASE for values
  ```cpp
  enum class EntityState {
      ACTIVE,
      PAUSED,
      DESTROYED
  };
  ```

**Code Formatting:**

- Indentation: 4 spaces (no tabs)
- Line length: 120 characters maximum
- Braces: Opening brace on same line
  ```cpp
  void Function() {
      // code
  }
  ```

- Include order:
  1. Corresponding header
  2. System headers
  3. Third-party headers
  4. Project headers

**Example:**

```cpp
#include "MyClass.h"

#include <iostream>
#include <vector>

#include <GLFW/glfw3.h>
#include <box2d/box2d.h>

#include <SAGE/ECS/Registry.h>
#include <SAGE/Graphics/Renderer.h>

namespace SAGE {

class MyComponent {
public:
    MyComponent(float initialValue)
        : value(initialValue), isActive(true) {
    }

    void Update(float deltaTime) {
        if (!isActive) {
            return;
        }
        
        value += deltaTime;
    }

private:
    float value;
    bool isActive;
};

} // namespace SAGE
```

### Best Practices

**Memory Management:**
- Use RAII principles
- Prefer smart pointers over raw pointers
- Avoid manual memory management when possible

**Performance:**
- Profile before optimizing
- Use const references for large objects
- Minimize heap allocations in hot paths
- Cache-friendly data structures

**Error Handling:**
- Return nullptr for failed resource loading
- Use bool returns for success/failure operations
- Log errors appropriately using SAGE logging macros
- Validate input parameters

**Comments:**
- Write self-documenting code
- Comment "why", not "what"
- Use Doxygen-style comments for public APIs

```cpp
/**
 * @brief Updates the physics simulation for one time step
 * @param registry The ECS registry containing entities
 * @param deltaTime Time elapsed since last update in seconds
 * @note This function must be called from the main thread only
 */
void UpdatePhysics(Registry& registry, float deltaTime);
```

## Submitting Changes

### Pull Request Guidelines

**Before submitting:**

1. Ensure code compiles without warnings
2. Run all tests and verify they pass
3. Update documentation if needed
4. Follow commit message conventions

**Commit Messages:**

Use clear, descriptive commit messages following this format:

```
[Type] Brief description (50 chars or less)

More detailed explanation if needed (wrap at 72 characters).
Explain the problem this commit solves and why the change was made.

Fixes #123
```

**Types:**
- `[Feature]` - New feature
- `[Fix]` - Bug fix
- `[Refactor]` - Code refactoring
- `[Docs]` - Documentation changes
- `[Test]` - Adding or updating tests
- `[Perf]` - Performance improvement
- `[Style]` - Code style/formatting

**Example:**
```
[Feature] Add sprite animation system

Implements a component-based sprite animation system with
support for frame-based animations and animation events.

- Add AnimationComponent with configurable frame rates
- Add AnimationSystem for updating animations
- Add animation state machine support
- Include 5 unit tests for animation logic

Fixes #42
```

### Pull Request Template

Your PR should include:

- **Title:** Clear, descriptive title
- **Description:** What changes were made and why
- **Type:** Feature / Bug Fix / Refactor / Documentation
- **Testing:** How the changes were tested
- **Breaking Changes:** Any breaking API changes
- **Related Issues:** Link to related issues

## Testing Guidelines

### Writing Tests

All new features and bug fixes should include tests:

1. **Unit Tests** - Test individual components/functions
2. **Integration Tests** - Test system interactions
3. **Performance Tests** - For performance-critical code

**Test Structure:**

```cpp
#include <catch2/catch_test_macros.hpp>
#include <SAGE/ECS/Registry.h>

TEST_CASE("Registry creates valid entities", "[ecs][registry]") {
    SAGE::ECS::Registry registry;
    
    SECTION("Single entity creation") {
        auto entity = registry.CreateEntity();
        REQUIRE(registry.IsValid(entity));
    }
    
    SECTION("Multiple entity creation") {
        auto e1 = registry.CreateEntity();
        auto e2 = registry.CreateEntity();
        REQUIRE(e1 != e2);
        REQUIRE(registry.IsValid(e1));
        REQUIRE(registry.IsValid(e2));
    }
}
```

**Test Categories:**

Use appropriate tags:
- `[ecs]` - ECS system tests
- `[physics]` - Physics tests
- `[graphics]` - Rendering tests
- `[audio]` - Audio system tests
- `[integration]` - Integration tests
- `[performance]` - Performance tests

### Running Tests

Run all tests:
```bash
./build/bin/Debug/SAGETests
```

Run specific category:
```bash
./build/bin/Debug/SAGETests [ecs]
```

Run with verbose output:
```bash
./build/bin/Debug/SAGETests --success
```

## Documentation

### Code Documentation

Use Doxygen-style comments for:
- Public classes
- Public methods
- Public functions
- Complex algorithms

### User Documentation

When adding features that affect users:

1. Update relevant documentation in `docs/`
2. Add examples if appropriate
3. Update API reference
4. Include usage examples

### Documentation Style

- Use clear, concise language
- Provide code examples
- Include performance considerations
- Note thread-safety requirements
- Document error conditions

## Questions?

If you have questions about contributing:

1. Check existing documentation in `docs/`
2. Search existing issues and pull requests
3. Open a new issue with the `question` label
4. Join our community discussions

## License

By contributing to SAGE Engine, you agree that your contributions will be licensed under the MIT License.

Thank you for contributing to SAGE Engine!
