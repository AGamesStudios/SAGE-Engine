# 🗂️ SAGE Engine - Quick Navigation Index

> Fast access to all important files and directories

---

## 🚀 Most Important Files

### Start Here
- [📖 README.md](../README.md) - **Project overview and quick start**
- [📁 STRUCTURE.md](markdown/STRUCTURE.md) - **Detailed project structure**
- [🎨 VISUAL GUIDE](PROJECT_STRUCTURE_VISUAL.md) - **Visual structure overview**

### Documentation Hub
- [📚 Documentation Index](README.md) - **All documentation**

---

## 🔧 Development

### Build & Setup
- [🛠️ tools/install.bat](../tools/install.bat) - Windows installation
- [🛠️ tools/install.sh](../tools/install.sh) - Linux/macOS installation
- [⚙️ CMakeLists.txt](../CMakeLists.txt) - Main build config

### IDE Configuration
- [💻 .vscode/settings.json](../.vscode/settings.json) - VS Code settings
- [▶️ .vscode/tasks.json](../.vscode/tasks.json) - Build tasks
- [🐛 .vscode/launch.json](../.vscode/launch.json) - Debug config

---

## 📚 Documentation

### Core Docs (`docs/markdown/`)
- [📝 CHANGELOG.md](markdown/CHANGELOG.md) - Version history
- [🤝 CONTRIBUTING.md](markdown/CONTRIBUTING.md) - How to contribute
- [⚖️ LICENSE](markdown/LICENSE) - MIT License
- [📁 STRUCTURE.md](markdown/STRUCTURE.md) - Project structure

### Guides (`docs/guides/`)
- [🎓 GETTING_STARTED.md](guides/GETTING_STARTED.md) - Beginner tutorial
- [⚡ QUICKSTART.md](guides/QUICKSTART.md) - Quick setup
- [💻 INSTALL.md](guides/INSTALL.md) - Installation guide
- [📖 EXAMPLES.md](guides/EXAMPLES.md) - Code examples

### Project Info (`docs/project/`)
- [🗺️ ROADMAP.md](project/ROADMAP.md) - Future plans
- [📊 PROJECT_STATUS.md](project/PROJECT_STATUS.md) - Current status

---

## 🎮 Source Code

### Engine (`Engine/`)
| System | Location | Description |
|--------|----------|-------------|
| Core | `Engine/Core/` | Application, Window, Scene, GameObject |
| Graphics | `Engine/Graphics/` | Renderer, Shader, Texture, Sprite |
| Physics | `Engine/Physics/` | Collision, Raycast, Forces |
| Audio | `Engine/Audio/` | Sound system |
| Input | `Engine/Input/` | Keyboard, Mouse, Gamepad |
| Resources | `Engine/Resources/` | Asset management |
| UI | `Engine/UI/` | Widgets, Buttons, Panels |
| Math | `Engine/Math/` | Vector2, Rect, Transform |
| Memory | `Engine/Memory/` | Vault allocator |

### Examples (`Examples/`)
- [🎮 SimpleGame/](../Examples/SimpleGame/) - Basic game example
- [🎪 Demo/](../Examples/Demo/) - Feature showcase

### Tests (`Tests/`)
- [🧪 Tests/](../Tests/) - All unit and integration tests

---

## 📦 External

### Dependencies (`ThirdParty/`)
- [🪟 glfw/](../ThirdParty/glfw/) - Window & input library
- [✨ glad/](../ThirdParty/glad/) - OpenGL loader

### Assets (`Assets/`)
- [🖼️ textures/](../Assets/textures/) - Images
- [🔊 audio/](../Assets/audio/) - Sounds
- [🔤 fonts/](../Assets/fonts/) - Fonts
- [🎨 shaders/](../Assets/shaders/) - GLSL shaders

---

## 🏗️ Build Output

### Build Directory (`build/`)
> ⚠️ This directory is auto-generated and gitignored

- `build/Engine/` - Compiled engine library
- `build/Examples/` - Example executables
- `build/Tests/` - Test executables
- `build/ThirdParty/` - Built dependencies

---

## 📂 Directory Tree

```
SAGE-Engine/
├── Engine/           → Core engine source
├── Examples/         → Example projects
├── Tests/            → Test suite
├── ThirdParty/       → Dependencies
├── Assets/           → Game resources
├── docs/             → Documentation
├── tools/            → Build scripts
├── build/            → Build output
├── .github/          → CI/CD
├── .vscode/          → VS Code config
├── .project/         → Project metadata
└── Root files        → CMake, README, gitignore
```

---

## 🔎 Search Tips

### Find by Topic
- **Getting Started** → `docs/guides/GETTING_STARTED.md`
- **API Reference** → `docs/api/` (future)
- **Examples** → `docs/guides/EXAMPLES.md` or `Examples/`
- **Contributing** → `docs/markdown/CONTRIBUTING.md`
- **Build Issues** → `docs/guides/INSTALL.md`

### Find by File Type
- **Markdown docs** → `docs/markdown/*.md`
- **Source code** → `Engine/**/*.cpp` or `Engine/**/*.h`
- **Examples** → `Examples/*/src/`
- **Tests** → `Tests/*Tests.cpp`
- **Scripts** → `tools/*.bat` or `tools/*.sh`

---

## 💡 Tips

1. **Start with README.md** for project overview
2. **Check docs/README.md** for documentation hub
3. **Use STRUCTURE.md** to understand organization
4. **Browse Examples/** for code samples
5. **Read CONTRIBUTING.md** before submitting PRs

---

## 📞 Quick Links

| What | Where |
|------|-------|
| **Report Bug** | [GitHub Issues](https://github.com/AGamesStudios/SAGE-Engine/issues) |
| **Ask Question** | [Discussions](https://github.com/AGamesStudios/SAGE-Engine/discussions) |
| **Contribute** | [CONTRIBUTING.md](markdown/CONTRIBUTING.md) |
| **License** | [LICENSE](markdown/LICENSE) |
| **Changelog** | [CHANGELOG.md](markdown/CHANGELOG.md) |

---

**Last Updated**: October 6, 2025
