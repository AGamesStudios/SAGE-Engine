# SAGE CLI - Quick Reference

## Installation

**Install SAGE CLI:**
```bash
cd SAGE-Engine/tools
python install_cli.py
```

**Verify:**
```bash
sage --version
```

---

## Engine Commands

| Command | Description | Example |
|---------|-------------|---------|
| `sage install` | Install SAGE Engine | `sage install` |
| `sage build` | Build engine | `sage build --config Release` |
| `sage test` | Run tests | `sage test --filter "[ecs]"` |
| `sage clean` | Clean build | `sage clean --deep` |
| `sage info` | Show engine info | `sage info` |

---

## Project Commands

| Command | Description | Example |
|---------|-------------|---------|
| `sage create <name>` | Create new project | `sage create MyGame` |
| `sage list` | List all projects | `sage list` |
| `sage project build` | Build current project | `sage project build` |
| `sage project run` | Run current project | `sage project run` |
| `sage project clean` | Clean project | `sage project clean` |

---

## Quick Workflows

### First Time Setup
```bash
# Install CLI
cd SAGE-Engine/tools
python install_cli.py

# Install engine
sage install

# Run tests
sage test
```

### Create and Run Game
```bash
# Create project
sage create MyAwesomeGame

# Navigate to project
cd ../SAGEProjects/MyAwesomeGame

# Build
sage project build

# Run
sage project run
```

### Development Cycle
```bash
# Edit code...

# Build
sage project build --config Debug

# Run
sage project run --config Debug
```

---

## Common Options

**Build Configurations:**
- `--config Debug` - Debug build with symbols
- `--config Release` - Optimized release build

**Test Filters:**
- `--filter "[ecs]"` - Run ECS tests
- `--filter "[physics]"` - Run physics tests
- `--filter "[graphics]"` - Run graphics tests
- `--verbose` - Detailed output

**Parallel Building:**
- `--parallel 4` - Use 4 parallel jobs
- `--parallel 8` - Use 8 parallel jobs

---

## Examples

**Build engine in Debug mode:**
```bash
sage build --config Debug --parallel 8
```

**Run specific tests:**
```bash
sage test --filter "[physics]" --verbose
```

**Create and build project:**
```bash
sage create SpaceShooter
cd ../SAGEProjects/SpaceShooter
sage project build --config Release
sage project run
```

**Full clean and rebuild:**
```bash
sage clean --deep
sage install
sage test
```

---

## Help

Get help for any command:
```bash
sage --help
sage build --help
sage project --help
```

Full documentation: [docs/CLI_GUIDE.md](../docs/CLI_GUIDE.md)

---

**SAGE Engine CLI - Simplifying game development workflow**
