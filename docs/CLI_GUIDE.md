# SAGE Engine CLI Guide

**Version:** Alpha  
**Tool:** sage  
**Language:** Python 3.6+

---

## Overview

SAGE CLI is a command-line interface for managing SAGE Engine installation, builds, tests, and projects. It simplifies common development tasks and project management.

## Installation

### Prerequisites

- Python 3.6 or higher
- Git
- CMake 3.15+
- C++17 compiler (Visual Studio 2022, GCC 9+, or Clang 10+)

### Install SAGE CLI

**Windows:**
```powershell
cd SAGE-Engine/tools
python install_cli.py
```

**Linux/macOS:**
```bash
cd SAGE-Engine/tools
python3 install_cli.py
```

The installer will:
1. Create `sage` command
2. Install to user scripts directory
3. Add to PATH (if needed)

### Verify Installation

```bash
sage --version
```

Expected output:
```
SAGE CLI 0.1.0-alpha
```

---

## Engine Commands

### Install Engine

Install and configure SAGE Engine with all dependencies:

```bash
sage install
```

**Options:**
- `--skip-build` - Skip building after installation

**What it does:**
1. Checks prerequisites (CMake, Git)
2. Initializes git submodules
3. Configures CMake build
4. Builds the engine (unless `--skip-build`)

**Example:**
```bash
# Full installation with build
sage install

# Installation without building
sage install --skip-build
```

### Build Engine

Build SAGE Engine from source:

```bash
sage build
```

**Options:**
- `--config {Debug|Release}` - Build configuration (default: Release)
- `--parallel N` - Number of parallel build jobs

**Examples:**
```bash
# Build Release (default)
sage build

# Build Debug
sage build --config Debug

# Build with 8 parallel jobs
sage build --parallel 8

# Debug build with parallel compilation
sage build --config Debug --parallel 4
```

### Run Tests

Execute SAGE Engine test suite:

```bash
sage test
```

**Options:**
- `--config {Debug|Release}` - Test configuration (default: Release)
- `--filter PATTERN` - Run specific tests matching pattern
- `--verbose` - Show detailed test output

**Examples:**
```bash
# Run all tests
sage test

# Run Debug tests
sage test --config Debug

# Run specific test category
sage test --filter "[ecs]"

# Run with verbose output
sage test --verbose

# Run physics tests in Debug mode
sage test --config Debug --filter "[physics]"
```

### Clean Build

Remove build artifacts:

```bash
sage clean
```

**Options:**
- `--deep` - Remove entire build directory

**Examples:**
```bash
# Clean build artifacts (keep CMake cache)
sage clean

# Complete clean (remove everything)
sage clean --deep
```

### Engine Info

Display SAGE Engine information and status:

```bash
sage info
```

**Output includes:**
- Engine version
- Installation paths
- Configuration settings
- Build status
- Available executables

---

## Project Commands

### Create New Project

Create a new SAGE Engine project with template:

```bash
sage create ProjectName
```

**What it creates:**
```
ProjectName/
├── CMakeLists.txt      # Build configuration
├── README.md           # Project documentation
├── .gitignore          # Git ignore file
├── src/
│   └── main.cpp        # Main application
└── assets/
    ├── textures/       # Image assets
    ├── audio/          # Sound assets
    └── shaders/        # Shader files
```

**Example main.cpp:**
```cpp
#include <SAGE/SAGE.h>

class MyGame : public SAGE::Application {
public:
    MyGame() : Application("My Game", 1280, 720) {}
    
    void OnInit() override {
        // Initialize game
    }
    
    void OnUpdate(float deltaTime) override {
        // Update game logic
    }
    
    void OnRender() override {
        // Render game
    }
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

**Examples:**
```bash
# Create simple game project
sage create MyGame

# Create platformer project
sage create PlatformerGame

# Create RPG project
sage create RPGAdventure
```

### List Projects

List all SAGE Engine projects:

```bash
sage list
```

Shows:
- Project name
- Location
- Build status

### Build Project

Build the current project (run from project directory):

```bash
cd MyProject
sage project build
```

**Options:**
- `--config {Debug|Release}` - Build configuration (default: Release)
- `--reconfigure` - Force CMake reconfiguration

**Examples:**
```bash
# Build Release
sage project build

# Build Debug
sage project build --config Debug

# Rebuild with reconfiguration
sage project build --reconfigure

# Debug build with reconfiguration
sage project build --config Debug --reconfigure
```

### Run Project

Run the current project executable:

```bash
cd MyProject
sage project run
```

**Options:**
- `--config {Debug|Release}` - Run configuration (default: Release)

**Examples:**
```bash
# Run Release build
sage project run

# Run Debug build
sage project run --config Debug
```

### Clean Project

Clean project build artifacts:

```bash
cd MyProject
sage project clean
```

Removes the entire `build/` directory.

---

## Complete Workflows

### First Time Setup

Complete installation and verification:

```bash
# Install SAGE CLI
cd SAGE-Engine/tools
python install_cli.py

# Verify installation
sage --version

# Install engine
sage install

# Run tests to verify
sage test

# Check engine info
sage info
```

### Creating and Running a New Game

Full workflow from creation to execution:

```bash
# Create new project
sage create MyAwesomeGame

# Navigate to project
cd ../SAGEProjects/MyAwesomeGame

# Edit src/main.cpp with your game code

# Build project
sage project build

# Run project
sage project run
```

### Development Workflow

Typical iterative development:

```bash
# Work on your code
# Edit src/main.cpp

# Build
sage project build --config Debug

# Run
sage project run --config Debug

# If issues, clean and rebuild
sage project clean
sage project build --config Debug
```

### Engine Development Workflow

Working on SAGE Engine itself:

```bash
# Make engine changes
# Edit Engine/...

# Rebuild engine
sage build --config Debug --parallel 8

# Run tests
sage test --config Debug --verbose

# Test specific component
sage test --filter "[ecs]"

# Clean if needed
sage clean
```

---

## Configuration

SAGE CLI stores configuration in `tools/sage_config.json`:

```json
{
  "default_compiler": "auto",
  "default_build_type": "Release",
  "projects_directory": "/path/to/SAGEProjects"
}
```

**Settings:**
- `default_compiler` - Default compiler selection
- `default_build_type` - Default build configuration
- `projects_directory` - Location for new projects

---

## Troubleshooting

### Command Not Found

**Problem:** `sage: command not found`

**Solution:**
1. Reinstall CLI: `python install_cli.py`
2. Add to PATH manually (see installation output)
3. Restart terminal

**Windows:**
```powershell
setx PATH "%PATH%;%USERPROFILE%\AppData\Local\Programs\Python\Scripts"
```

**Linux/macOS:**
```bash
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc
```

### Build Fails

**Problem:** Build fails with CMake errors

**Solutions:**
1. Clean and rebuild:
   ```bash
   sage clean --deep
   sage install
   ```

2. Check prerequisites:
   ```bash
   cmake --version  # Should be 3.15+
   git --version
   ```

3. Update submodules:
   ```bash
   cd SAGE-Engine
   git submodule update --init --recursive
   ```

### Tests Fail

**Problem:** Tests fail to run or some tests fail

**Solutions:**
1. Rebuild in Debug mode:
   ```bash
   sage build --config Debug
   sage test --config Debug --verbose
   ```

2. Run specific failing test:
   ```bash
   sage test --filter "[failing-test]" --verbose
   ```

3. Check test executable exists:
   ```bash
   sage info
   ```

### Project Won't Build

**Problem:** Project build fails

**Solutions:**
1. Reconfigure CMake:
   ```bash
   sage project build --reconfigure
   ```

2. Clean and rebuild:
   ```bash
   sage project clean
   sage project build
   ```

3. Ensure engine is built:
   ```bash
   cd SAGE-Engine
   sage build
   ```

---

## Advanced Usage

### Custom Build Directory

Work with non-standard build location:

```bash
# Create custom build
cmake -S . -B custom_build
cmake --build custom_build --config Release

# Note: SAGE CLI expects build/ directory
# For custom builds, use cmake directly
```

### Cross-Platform Projects

Project CMakeLists.txt is portable:

**Windows:**
```powershell
sage project build
sage project run
```

**Linux:**
```bash
sage project build
sage project run
```

Same commands work on all platforms.

### Multiple Configurations

Build and test both Debug and Release:

```bash
# Build both
sage build --config Debug
sage build --config Release

# Test both
sage test --config Debug
sage test --config Release
```

### Batch Operations

Build all projects:

```bash
# List all projects
sage list

# Navigate to each and build
cd ../SAGEProjects/Game1 && sage project build
cd ../Game2 && sage project build
cd ../Game3 && sage project build
```

---

## CLI Reference

### Global Options

- `--version` - Show SAGE CLI version
- `--help` - Show help message

### Command Summary

**Engine Management:**
```
sage install [--skip-build]              Install engine
sage build [--config] [--parallel]       Build engine
sage test [--config] [--filter] [--verbose]  Run tests
sage clean [--deep]                      Clean build
sage info                                Show information
```

**Project Management:**
```
sage create <name>                       Create project
sage list                                List projects
sage project build [--config] [--reconfigure]  Build project
sage project run [--config]              Run project
sage project clean                       Clean project
```

---

## Integration with IDEs

### Visual Studio Code

Add tasks to `.vscode/tasks.json`:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "SAGE: Build Project",
      "type": "shell",
      "command": "sage",
      "args": ["project", "build"],
      "group": {
        "kind": "build",
        "isDefault": true
      }
    },
    {
      "label": "SAGE: Run Project",
      "type": "shell",
      "command": "sage",
      "args": ["project", "run"],
      "group": {
        "kind": "test",
        "isDefault": true
      }
    }
  ]
}
```

### Visual Studio

Use Command Prompt integration:

**Tools → External Tools → Add:**
- Title: `SAGE Build`
- Command: `sage`
- Arguments: `project build`
- Initial directory: `$(ProjectDir)`

---

## Examples

### Quick Start Example

```bash
# Install everything
sage install

# Create a game
sage create SpaceShooter
cd ../SAGEProjects/SpaceShooter

# Build and run
sage project build
sage project run
```

### Testing Workflow

```bash
# Run all tests
sage test

# Run specific category
sage test --filter "[physics]"

# Debug failing test
sage test --config Debug --filter "[failing]" --verbose
```

### Release Build

```bash
# Clean everything
sage clean --deep

# Fresh Release build
sage install

# Verify with tests
sage test

# Build project for release
cd ../SAGEProjects/MyGame
sage project clean
sage project build --config Release
```

---

## Best Practices

1. **Always test after engine changes:**
   ```bash
   sage build
   sage test
   ```

2. **Use Debug for development:**
   ```bash
   sage project build --config Debug
   sage project run --config Debug
   ```

3. **Clean when switching configurations:**
   ```bash
   sage project clean
   sage project build --config Release
   ```

4. **Check engine status regularly:**
   ```bash
   sage info
   ```

5. **Keep projects organized:**
   - Use descriptive names
   - Run `sage list` to track projects
   - Keep projects in SAGEProjects directory

---

## Support

For issues with SAGE CLI:
1. Check `sage info` for status
2. Review troubleshooting section
3. Report bugs: https://github.com/AGamesStudios/SAGE-Engine/issues
4. Include CLI version: `sage --version`

---

## Summary

SAGE CLI provides a unified interface for:
- Installing and building SAGE Engine
- Running tests and verification
- Creating and managing game projects
- Streamlining development workflow

**Essential Commands:**
- `sage install` - Set up engine
- `sage create MyGame` - Start new project
- `sage project build` - Build your game
- `sage project run` - Play your game

Happy game development with SAGE Engine!
