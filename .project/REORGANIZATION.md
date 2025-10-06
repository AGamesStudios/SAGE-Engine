# 🎨 SAGE Engine - Clean Project Organization

This document describes the reorganized project structure implemented on October 6, 2025.

---

## ✨ What Changed?

### Moved Files
1. **Documentation** → `docs/markdown/`
   - `CHANGELOG.md` → `docs/markdown/CHANGELOG.md`
   - `CONTRIBUTING.md` → `docs/markdown/CONTRIBUTING.md`
   - `LICENSE` → `docs/markdown/LICENSE`
   - `STRUCTURE.md` → `docs/markdown/STRUCTURE.md`

2. **Scripts** → `tools/`
   - `scripts/install.bat` → `tools/install.bat`
   - `scripts/install.sh` → `tools/install.sh`
   - `scripts/` directory removed

3. **Demo** → `Examples/`
   - `Demo/` → `Examples/Demo/`

### New Directories
- `docs/markdown/` - All markdown documentation
- `docs/api/` - API documentation (future)
- `tools/` - Build scripts and utilities
- `.project/` - Project metadata (future use)

---

## 📁 New Structure Overview

```
SAGE-Engine/
├── Engine/              ✅ Core engine (unchanged)
├── Examples/            ✅ All examples including Demo
├── Tests/               ✅ Unit tests (unchanged)
├── ThirdParty/          ✅ Dependencies (unchanged)
├── Assets/              ✅ Game assets (unchanged)
├── docs/                🆕 Organized documentation
│   ├── markdown/        🆕 MD docs (CHANGELOG, LICENSE, etc.)
│   ├── api/             🆕 API documentation
│   ├── guides/          ✅ User guides
│   └── project/         ✅ Project docs
├── tools/               🆕 Build scripts (was scripts/)
├── build/               ✅ Build output
├── .github/             ✅ GitHub configs
├── .vscode/             ✅ VS Code settings
├── CMakeLists.txt       ✅ Main build file
└── README.md            ✅ Updated links
```

---

## 🎯 Benefits

### Better Organization
- All documentation in one place (`docs/`)
- Clear separation of concerns
- Easier to navigate

### Cleaner Root
- Less clutter in root directory
- Professional appearance
- Logical grouping

### Improved Discoverability
- Documentation hub at `docs/README.md`
- Centralized markdown files
- Clear naming conventions

---

## 🔄 Migration Guide

### Updating Links
If you had links to old paths, update them:

```markdown
# Old → New
LICENSE → docs/markdown/LICENSE
CHANGELOG.md → docs/markdown/CHANGELOG.md
CONTRIBUTING.md → docs/markdown/CONTRIBUTING.md
STRUCTURE.md → docs/markdown/STRUCTURE.md
scripts/install.bat → tools/install.bat
scripts/install.sh → tools/install.sh
```

### CMake Updates
No CMake updates needed - all source paths remain unchanged!

### Git History
All files maintain their git history through `git mv` operations.

---

## 📝 Naming Conventions

### Directories
- **Lowercase**: `docs`, `tools`, `build`
- **PascalCase**: `Engine`, `Examples`, `Tests`, `ThirdParty`, `Assets`
- **Hidden**: `.github`, `.vscode`, `.project`

### Files
- **Markdown**: `UPPERCASE.md` for docs (e.g., `README.md`, `CHANGELOG.md`)
- **Code**: `PascalCase.h/cpp` (e.g., `Application.h`, `Renderer.cpp`)
- **Config**: lowercase or standard names (e.g., `CMakeLists.txt`, `.gitignore`)

---

## 🎨 Visual Structure

```
🎮 SAGE-Engine/
│
├── 🔧 Core Components
│   ├── Engine/        → Engine source code
│   ├── Examples/      → Sample projects
│   └── Tests/         → Test suite
│
├── 📦 External
│   ├── ThirdParty/    → Dependencies
│   └── Assets/        → Game resources
│
├── 📚 Documentation
│   └── docs/          → All documentation
│       ├── markdown/  → Core docs
│       ├── api/       → API reference
│       ├── guides/    → Tutorials
│       └── project/   → Project info
│
├── 🛠️ Development
│   ├── tools/         → Build scripts
│   ├── build/         → Compiled output
│   ├── .github/       → CI/CD
│   └── .vscode/       → Editor config
│
└── 📄 Root Files
    ├── CMakeLists.txt → Build config
    ├── README.md      → Project overview
    └── .gitignore     → Git config
```

---

## ✅ Verification

After reorganization, verify:

1. **Build still works**:
   ```powershell
   cmake -S . -B build -G "Visual Studio 17 2022"
   cmake --build build --config Release
   ```

2. **All documentation accessible**:
   - Check `docs/README.md`
   - Verify all links work

3. **Git history preserved**:
   ```powershell
   git log --follow docs/markdown/CHANGELOG.md
   ```

---

## 🚀 Future Improvements

- [ ] Auto-generate API documentation to `docs/api/`
- [ ] Add more guides to `docs/guides/`
- [ ] Create developer setup scripts in `tools/`
- [ ] Organize build outputs better in `build/`

---

**Reorganized**: October 6, 2025  
**Version**: 2.0.0 (rewrite/sage2d branch)
