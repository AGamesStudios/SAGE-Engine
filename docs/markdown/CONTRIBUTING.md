# Contributing to SAGE Engine

Thank you for your interest in contributing to SAGE Engine! This document provides guidelines and instructions for contributing.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Contributing Guidelines](#contributing-guidelines)
- [Pull Request Process](#pull-request-process)
- [Coding Standards](#coding-standards)
- [Testing](#testing)
- [Documentation](#documentation)
- [Community](#community)

---

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inspiring community for all. Please be respectful and constructive in all interactions.

### Our Standards

**Positive behavior includes:**
- Using welcoming and inclusive language
- Being respectful of differing viewpoints
- Gracefully accepting constructive criticism
- Focusing on what is best for the community
- Showing empathy towards other community members

**Unacceptable behavior includes:**
- Harassment, trolling, or derogatory comments
- Publishing others' private information
- Other conduct which could reasonably be considered inappropriate

---

## Getting Started

### Ways to Contribute

- 🐛 **Bug Reports**: Report bugs through GitHub Issues
- ✨ **Feature Requests**: Suggest new features or improvements
- 📝 **Documentation**: Improve or fix documentation
- 💻 **Code**: Submit bug fixes or new features
- 🎨 **Examples**: Create example projects
- 🧪 **Testing**: Write or improve tests

### Before You Start

1. Check if the issue already exists in [GitHub Issues](https://github.com/AGamesStudios/SAGE-Engine/issues)
2. For major changes, open an issue first to discuss your approach
3. Fork the repository and create a branch from `main`
4. Review the coding standards and project structure

---

## Development Setup

### Prerequisites

- **Git**: Version control
- **CMake**: 3.20 or higher
- **Compiler**: MSVC 2019+, GCC 9+, or Clang 10+
- **OpenGL**: 3.3 or higher

### Setup Instructions

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/SAGE-Engine.git
   cd SAGE-Engine
   ```

2. **Create a Branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/issue-number-description
   ```

3. **Build the Project**
   ```bash
   mkdir build && cd build
   cmake .. -G "Visual Studio 17 2022"
   cmake --build . --config Debug
   ```

4. **Run Tests**
   ```bash
   ctest -C Debug -VV
   # or
   .\Tests\Debug\SAGE_Tests.exe
   ```

### Project Structure

```
SAGE-Engine/
├── Engine/              # Core engine code
│   ├── Audio/          # Audio system
│   ├── Core/           # Core systems
│   ├── Graphics/       # Rendering
│   ├── Input/          # Input handling
│   ├── Math/           # Math utilities
│   ├── Physics/        # Physics system
│   ├── Resources/      # Resource management
│   ├── UI/             # UI system
│   └── SAGE.h          # Main header
├── Examples/           # Example projects
├── Tests/              # Unit tests
├── ThirdParty/         # External dependencies
└── scripts/            # Build and install scripts
```

---

## Contributing Guidelines

### Bug Reports

When reporting a bug, include:

- **Clear Title**: Descriptive summary of the issue
- **Environment**: OS, compiler, SAGE version
- **Steps to Reproduce**: Detailed steps to reproduce the bug
- **Expected Behavior**: What you expected to happen
- **Actual Behavior**: What actually happened
- **Code Sample**: Minimal reproducible example
- **Screenshots**: If applicable

**Example:**
```markdown
## Bug: Renderer crashes on window resize

**Environment:**
- OS: Windows 11
- Compiler: MSVC 2022
- SAGE Version: 0.1.0

**Steps to Reproduce:**
1. Create application with Renderer
2. Resize window rapidly
3. Application crashes

**Expected:** Window resizes smoothly
**Actual:** Access violation in Renderer::Update()

**Code:**
\`\`\`cpp
// Minimal example showing the issue
\`\`\`
```

### Feature Requests

When requesting a feature, include:

- **Clear Description**: What you want to add/change
- **Use Case**: Why this feature is needed
- **Proposed Solution**: How you think it should work
- **Alternatives**: Other solutions you've considered
- **Additional Context**: Any other relevant information

### Code Contributions

1. **Write Clean Code**: Follow coding standards below
2. **Add Tests**: Write tests for new functionality
3. **Update Documentation**: Document new features/changes
4. **Keep Changes Focused**: One feature/fix per PR
5. **Test Thoroughly**: Ensure all tests pass

---

## Pull Request Process

### Before Submitting

- [ ] Code follows project coding standards
- [ ] All tests pass locally
- [ ] New tests added for new functionality
- [ ] Documentation updated (if applicable)
- [ ] Commit messages are clear and descriptive
- [ ] Branch is up-to-date with `main`

### Submitting a Pull Request

1. **Push your changes**
   ```bash
   git push origin feature/your-feature-name
   ```

2. **Create Pull Request** on GitHub with:
   - **Title**: Clear, descriptive summary
   - **Description**: What changes were made and why
   - **Related Issues**: Link to relevant issues
   - **Testing**: How you tested the changes
   - **Screenshots**: If UI changes

3. **Respond to Feedback**: Address review comments promptly

4. **Wait for Review**: Maintainers will review your PR

### PR Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Related Issues
Fixes #(issue number)

## Testing
How has this been tested?

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-reviewed code
- [ ] Commented hard-to-understand areas
- [ ] Documentation updated
- [ ] No new warnings
- [ ] Tests added
- [ ] All tests pass
```

---

## Coding Standards

### C++ Style Guide

#### Naming Conventions

```cpp
// Classes: PascalCase
class MyClass { };
class PhysicsSystem { };

// Functions/Methods: PascalCase
void DoSomething();
void UpdatePhysics();

// Variables: camelCase
int playerHealth;
float deltaTime;

// Member variables: m_ prefix
class Player {
    int m_Health;
    float m_Speed;
};

// Constants: UPPER_CASE
const int MAX_PLAYERS = 4;
constexpr float PI = 3.14159f;

// Enums: PascalCase
enum class State {
    Idle,
    Running,
    Jumping
};

// Namespaces: PascalCase
namespace SAGE { }
```

#### Code Formatting

```cpp
// Braces: New line for functions/classes, same line for control flow
class MyClass
{
public:
    void MyFunction()
    {
        if (condition) {
            // Code
        } else {
            // Code
        }
        
        for (int i = 0; i < 10; i++) {
            // Code
        }
    }
};

// Indentation: 4 spaces (no tabs)
void Example()
{
    if (true) {
        DoSomething();
    }
}

// Line length: Maximum 120 characters
// Break long lines logically

// Include order:
// 1. Corresponding header
// 2. Engine headers
// 3. Third-party headers
// 4. Standard library headers
#include "MyClass.h"
#include "Core/Logger.h"
#include <glad/glad.h>
#include <vector>
```

#### Best Practices

```cpp
// Use smart pointers
std::unique_ptr<Texture> texture;
std::shared_ptr<Sound> sound;

// Prefer const correctness
void ProcessData(const std::string& name) const;

// Use auto where type is obvious
auto texture = CreateTexture("player.png");
auto position = GetPosition();

// Avoid raw pointers (except for non-owning references)
GameObject* GetParent(); // OK - non-owning
Texture* LoadTexture(); // Bad - who owns it?

// Use nullptr instead of NULL
GameObject* obj = nullptr;

// Prefer enum class over enum
enum class State { Idle, Running };

// Use forward declarations to minimize includes
class Renderer; // Forward declaration
```

### Comments

```cpp
// Use /// for documentation comments
/// @brief Brief description
/// @param name Parameter description
/// @return Return value description
int CalculateScore(int points);

// Use // for inline comments
// This is a single-line comment

/* Multi-line comments
   for complex explanations */

// TODO: Feature to implement
// FIXME: Bug that needs fixing
// NOTE: Important information
```

---

## Testing

### Writing Tests

Tests are located in the `Tests/` directory using a custom test framework.

```cpp
#include "TestFramework.h"

TEST(MyFeature_BasicFunctionality)
{
    // Arrange
    MyClass obj;
    
    // Act
    int result = obj.Calculate(5);
    
    // Assert
    REQUIRE(result == 25, "Calculate should return square");
}
```

### Running Tests

```bash
# Run all tests
cd build
ctest -C Debug -VV

# Run specific test
.\Tests\Debug\SAGE_Tests.exe

# Run with profiling
ctest -C Debug -VV --output-on-failure
```

### Test Coverage

- Write tests for all new features
- Aim for high code coverage (>80%)
- Test edge cases and error conditions
- Keep tests isolated and independent

---

## Documentation

### Code Documentation

- Document all public APIs with `///` comments
- Include parameter descriptions and return values
- Provide usage examples for complex features
- Keep documentation up-to-date with code changes

### README Updates

Update README.md if you:
- Add major features
- Change build process
- Update system requirements
- Add new dependencies

### Example Projects

When adding examples:
- Create in `Examples/YourExample/`
- Include `CMakeLists.txt`
- Add `README.md` explaining the example
- Update `EXAMPLES.md` with tutorial
- Keep code well-commented

---

## Community

### Getting Help

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and community discussion
- **Discord**: Real-time chat (link in README)
- **Documentation**: Comprehensive guides and API docs

### Recognition

Contributors will be:
- Listed in CONTRIBUTORS.md
- Mentioned in release notes
- Credited in relevant documentation

---

## Release Process

### Versioning

SAGE uses Semantic Versioning (semver):
- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes

### Changelog

All notable changes are documented in CHANGELOG.md following [Keep a Changelog](https://keepachangelog.com/) format.

---

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

## Questions?

If you have questions about contributing:
1. Check existing documentation
2. Search GitHub Issues/Discussions
3. Ask on Discord
4. Open a new Discussion

Thank you for contributing to SAGE Engine! 🎮

---

<div align="center">

Made with ❤️ by the SAGE Engine community

[⬆ Back to top](#contributing-to-sage-engine)

</div>
