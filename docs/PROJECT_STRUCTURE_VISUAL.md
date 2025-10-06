# 🎨 SAGE Engine - Project Structure Visual Guide

```
SAGE-Engine/
│
├── 🎮 ENGINE & CORE
│   ├── Engine/                    Core engine source code
│   │   ├── Core/                  Application, Window, Logger, Scene
│   │   ├── Graphics/              Renderer, Shader, Texture, Sprite
│   │   ├── Physics/               Collision, Raycast, Forces
│   │   ├── Audio/                 Sound system
│   │   ├── Input/                 Keyboard, Mouse, Gamepad
│   │   ├── Resources/             Asset management
│   │   ├── UI/                    UI widgets and system
│   │   ├── Math/                  Vector2, Rect, Transform
│   │   ├── Memory/                Custom allocators
│   │   └── sage2d/                2D features
│   │
│   ├── Examples/                  Example projects
│   │   ├── SimpleGame/            Basic game example
│   │   └── Demo/                  Feature showcase
│   │
│   └── Tests/                     Unit and integration tests
│
├── 📦 EXTERNAL & RESOURCES
│   ├── ThirdParty/                External dependencies
│   │   ├── glfw/                  Window & input library
│   │   └── glad/                  OpenGL loader
│   │
│   └── Assets/                    Shared game assets
│       ├── textures/              Image files
│       ├── audio/                 Sound files
│       ├── fonts/                 Font files
│       └── shaders/               GLSL shaders
│
├── 📚 DOCUMENTATION
│   └── docs/                      All documentation
│       ├── README.md              Documentation hub
│       ├── markdown/              Core documentation
│       │   ├── CHANGELOG.md       Version history
│       │   ├── CONTRIBUTING.md    Contribution guide
│       │   ├── LICENSE            MIT License
│       │   └── STRUCTURE.md       Project structure
│       ├── api/                   API documentation
│       ├── guides/                User tutorials
│       │   ├── GETTING_STARTED.md
│       │   ├── EXAMPLES.md
│       │   └── INSTALL.md
│       └── project/               Project info
│           ├── ROADMAP.md
│           └── PROJECT_STATUS.md
│
├── 🛠️ DEVELOPMENT TOOLS
│   ├── tools/                     Build scripts
│   │   ├── install.bat            Windows install script
│   │   └── install.sh             Unix install script
│   │
│   ├── build/                     Build output (gitignored)
│   │   ├── Engine/                Compiled engine
│   │   ├── Examples/              Compiled examples
│   │   ├── Tests/                 Compiled tests
│   │   └── ThirdParty/            Built dependencies
│   │
│   ├── .github/                   GitHub config
│   │   └── workflows/             CI/CD pipelines
│   │
│   ├── .vscode/                   VS Code settings
│   │   ├── settings.json          Editor config
│   │   ├── tasks.json             Build tasks
│   │   └── launch.json            Debug config
│   │
│   └── .project/                  Project metadata
│       └── REORGANIZATION.md      Reorganization notes
│
└── 📄 ROOT FILES
    ├── CMakeLists.txt             Main build configuration
    ├── README.md                  Project overview
    └── .gitignore                 Git ignore patterns

```

---

## 📊 Directory Statistics

| Category | Directories | Purpose |
|----------|-------------|---------|
| **Core** | 3 | Engine, Examples, Tests |
| **External** | 2 | ThirdParty, Assets |
| **Documentation** | 4 | docs/, markdown/, guides/, project/ |
| **Development** | 4 | tools/, build/, .github/, .vscode/ |
| **Hidden** | 2 | .project/, .venv/ |

---

## 🎯 Organization Principles

### 1. **Separation of Concerns**
- Engine code separate from examples
- Documentation isolated from source
- Build artifacts in dedicated directory

### 2. **Clear Hierarchy**
- Top-level: major components only
- Deep nesting: grouped by functionality
- Flat structure where possible

### 3. **Professional Layout**
- Clean root directory
- Standard naming conventions
- Intuitive navigation

### 4. **Developer-Friendly**
- Quick access to important files
- Logical grouping
- Self-documenting structure

---

## 🔍 Quick Find Guide

| Looking for... | Go to... |
|----------------|----------|
| Engine source | `Engine/` |
| Example code | `Examples/` |
| Tests | `Tests/` |
| Documentation | `docs/` |
| Build scripts | `tools/` |
| Dependencies | `ThirdParty/` |
| Assets | `Assets/` |
| Build output | `build/` |
| Version history | `docs/markdown/CHANGELOG.md` |
| How to contribute | `docs/markdown/CONTRIBUTING.md` |
| License info | `docs/markdown/LICENSE` |

---

## ✨ Key Features

### ✅ Clean Root
Only 3 files in root:
- `CMakeLists.txt` - Build config
- `README.md` - Project overview
- `.gitignore` - Git config

### ✅ Organized Documentation
All docs in `docs/` with clear subcategories

### ✅ Logical Grouping
Related files together (e.g., all build tools in `tools/`)

### ✅ Standard Conventions
Follows industry-standard project layout

---

**Last Updated**: October 6, 2025  
**Version**: 2.0.0 (rewrite/sage2d branch)
