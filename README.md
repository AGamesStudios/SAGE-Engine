# SAGE Engine - Simple And Game Engine

<div align="center">

![SAGE Engine](https://img.shields.io/badge/SAGE-Engine-v0.1.0_Alpha-blue)
![C++](https://img.shields.io/badge/C++-20-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![OpenGL](https://img.shields.io/badge/OpenGL-4.5-green.svg)

**Современный 2D игровой движок на C++20**

[Features](#features) • [Quick Start](#quick-start) • [Documentation](#documentation) • [Examples](#examples) • [Building](#building) • [Download](#download)

</div>

---

## 🎮 Features

### Core Systems
- ✅ **Modern Architecture** - ECS-inspired design with modular components
- ✅ **Scene Management** - Scene stack, transitions, and lifecycle management
- ✅ **Resource Management** - Automatic caching and cleanup
- ✅ **Plugin System** - Dynamic loading of DLL/SO plugins
- ✅ **Event System** - Type-safe global event bus
- ✅ **Save System** - JSON-based save/load with slots

### Graphics
- ✅ **OpenGL 4.5** - Modern rendering pipeline
- ✅ **Sprite Batching** - Efficient 2D rendering with automatic batching
- ✅ **Animation System** - Spritesheet animations with clips
- ✅ **Particle System** - Advanced particle emitters (Fire, Smoke, Rain, etc.)
- ✅ **Camera 2D** - With shake, smooth follow, and bounds
- ✅ **Tilemap** - Grid-based maps with layers and parallax
- ✅ **UV Coordinates** - Texture atlas and spritesheet support
- ✅ **Shader Loading** - From strings or files

### Physics & Math
- ✅ **Box2D Integration** - Full 2D physics simulation
- ✅ **QuadTree** - Spatial partitioning for efficient collision detection
- ✅ **Math Library** - Vector2, Matrix3, Rect, Color

### Performance
- ✅ **Profiler** - Built-in performance profiling with RAII macros
- ✅ **Timer System** - Delayed and repeating callbacks
- ✅ **FPS Tracking** - Real-time performance monitoring

### Audio
- ✅ **miniaudio** - Cross-platform audio playback
- ✅ **Sound Management** - Play, stop, loop, volume control

### Input
- ✅ **Keyboard** - Key press/release/hold detection
- ✅ **Mouse** - Button and position tracking
- ✅ **Gamepad** - Controller support (via GLFW)

### Tools
- ✅ **ImGui Integration** - Built-in debug UI
- ✅ **Logging** - Multi-level logging (Trace, Info, Warn, Error, Critical)
- ✅ **Dev Mode** - Runtime debugging tools

---

## 🚀 Quick Start

### Prerequisites
- **Visual Studio 2022** (or compatible C++20 compiler)
- **CMake 3.23+**
- **Git**

### Installation

```bash
# Clone the repository
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine

# Configure with CMake
cmake -S . -B build -G "Visual Studio 17 2022"

# Build
cmake --build build --config Release
```

### Your First Game

```cpp
#include <SAGE/SAGE.h>

using namespace SAGE;

class MyGame : public Game {
public:
    MyGame() : Game({.window = {.title = "My First Game"}}) {}

    void OnGameInit() override {
        // Load resources
        m_Texture = Texture::Create("assets/player.png");
    }

    void OnGameUpdate(float deltaTime) override {
        // Update game logic
        if (Input::IsKeyDown(Key::Escape)) {
            Quit();
        }
    }

    void OnGameRender() override {
        Renderer::Clear(Color::Black());
        Renderer::DrawQuad({100, 100}, {64, 64}, m_Texture.get());
    }

private:
    std::shared_ptr<Texture> m_Texture;
};

int main() {
    MyGame game;
    game.Run();
    return 0;
}
```

---

## 🎮 Примеры

В папке `Examples/` находятся два небольших демо, которые помогают быстро проверить отрисовку примитивов и настройку проекции:

1. **🖼 Pixel Canvas Demo** (`PixelCanvasDemo.cpp`)
    - Живая сцена с полосами, сеткой и HUD без 3D иллюзий
    - Демонстрирует пиксельную проекцию и разные толщины линий
    - Показывает, как комбинировать квадраты и линии для UI-оверлеев

2. **🟦 Shapes Demo** (`ShapesDemo.cpp`)
    - Анимированные квадраты, треугольник и многоугольник
    - Имитация осей координат и сетки
    - Использует `Renderer::DrawQuad`/`DrawLine`

**Руководство по разработке:** См. [docs/GAME_DEVELOPMENT_GUIDE.md](docs/GAME_DEVELOPMENT_GUIDE.md)

---

## 📚 Documentation

### Core Concepts

#### 1. Application Lifecycle

```cpp
class MyGame : public Game {
    void OnGameInit() override;      // Called once at startup
    void OnGameUpdate(float dt) override;  // Called every frame
    void OnGameRender() override;    // Called after update
    void OnGameShutdown() override;  // Called before exit
};
```

#### 2. Scene Management

```cpp
// Register scenes
SceneManager::Get().RegisterScene<MainMenuScene>("MainMenu");
SceneManager::Get().RegisterScene<GameScene>("Game");

// Switch scenes
SceneManager::Get().SwitchToScene("MainMenu");

// Push/Pop scenes (for overlays)
SceneManager::Get().PushScene("Pause");
SceneManager::Get().PopScene();
```

#### 3. Resource Management

```cpp
// Load and cache resources
auto texture = ResourceManager::Get().Load<Texture>("sprite.png");
auto shader = Shader::CreateFromFiles("vertex.glsl", "fragment.glsl");

// Resources are automatically cached and shared
```

#### 4. Particle Emitters

```cpp
// Create fire emitter
auto emitter = std::make_unique<ParticleEmitter>(500);
auto config = ParticleEmitter::CreateFireEmitter();
config.position = {100, 200};
emitter->SetConfig(config);
emitter->Start();

// Custom emitter
ParticleEmitterConfig custom;
custom.shape = EmitterShape::Cone;
custom.coneAngle = 45.0f;
custom.startColor = Color::Red();
custom.endColor = Color{1,1,1,0};
```

#### 5. Performance Profiling

```cpp
void MyFunction() {
    SAGE_PROFILE_FUNCTION();  // Automatically profiles this function
    
    {
        SAGE_PROFILE_SCOPE("Heavy Operation");
        // Your code here
    }
}

// Get results
auto results = Profiler::Get().GetResults();
for (const auto& result : results) {
    std::cout << result.name << ": " << result.averageMs << "ms\n";
}
```

#### 6. Spatial Partitioning

```cpp
// Create QuadTree
QuadTree<GameObject*> quadtree(
    Rect{0, 0, 1280, 720},
    10,  // Max objects per node
    5    // Max depth
);

// Insert objects
for (auto& obj : gameObjects) {
    quadtree.Insert({obj.bounds, &obj});
}

// Query nearby objects
auto nearby = quadtree.Retrieve(player.bounds);
for (auto* obj : nearby) {
    CheckCollision(player, *obj);
}
```

---

## 📖 Examples

### Pixel Canvas Demo
- Покрывает окно горизонтальными градиентами, сеткой и HUD
- Использует авто-настройку пиксельной проекции и разные толщины линий
- Подходит для отладки 2D-координат, цветов и анимаций

### Shapes Demo
- Примеры анимированных квадов и линий
- Демонстрирует изменение масштаба/угла на лету
- Удобно для отладки цвета, альфа и толщины линий

---

## 🔧 Building

### Windows (Visual Studio)

```powershell
# Configure
cmake -S . -B build -G "Visual Studio 17 2022"

# Build Debug
cmake --build build --config Debug

# Build Release
cmake --build build --config Release

# Run tests
.\build\bin\Debug\SAGE_Tests.exe
```

### CMake Options

```cmake
cmake -B build -DSAGE_BUILD_EXAMPLES=ON
-DSAGE_BUILD_TESTS=ON        # Build unit tests
-DSAGE_BUILD_EDITOR=ON       # Build editor (WIP)
```

---

## 🧪 Testing

```bash
# Run all tests
.\build\bin\Release\SAGE_Tests.exe

# Run specific test
.\build\bin\Release\SAGE_Tests.exe "Camera2D Tests"
```

Current test coverage: **~70%**

---

## 📦 Project Structure

```
SAGE-Engine/
├── Engine/              # Core engine library
│   ├── include/SAGE/   # Public headers
│   └── src/            # Implementation
├── Editor/             # Level editor (WIP)
├── Sandbox/            # Development testbed
├── Tests/              # Unit tests
├── Examples/           # Minimal render demos
├── ThirdParty/         # Dependencies
│   ├── glad/          # OpenGL loader
│   ├── glfw/          # Window management
│   ├── glm/           # Math library
│   ├── box2d/         # Physics
│   ├── imgui/         # UI
│   ├── miniaudio/     # Audio
│   └── stb_image/     # Image loading
└── assets/            # Resources

```

---

## 🎯 Roadmap

### ✅ Completed
- [x] Core rendering system
- [x] Scene management
- [x] Particle emitters
- [x] Performance profiler
- [x] Spatial partitioning
- [x] Shader from files
- [x] Multiple image formats (PNG, JPG, BMP, TGA)

### 🚧 In Progress
- [ ] Comprehensive API documentation
- [ ] More example projects
- [ ] Level editor

### 📋 Planned
- [ ] Networking (client/server)
- [ ] Mobile platforms (Android, iOS)
- [ ] Scripting (Lua integration)
- [ ] Advanced lighting (2D normal maps)

---

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style
- C++20 standard
- Use RAII and smart pointers
- Follow existing naming conventions
- Add comments for complex logic

---

## 📄 License

MIT License - see [LICENSE](LICENSE) file

---

## 🙏 Acknowledgments

- **GLFW** - Window and input handling
- **glad** - OpenGL function loading
- **GLM** - Math library
- **Box2D** - Physics engine
- **Dear ImGui** - UI framework
- **miniaudio** - Audio playback
- **STB** - Image loading
- **Catch2** - Unit testing

---

## 📞 Contact

- **GitHub**: [AGamesStudios/SAGE-Engine](https://github.com/AGamesStudios/SAGE-Engine)
- **Issues**: [Report Bug](https://github.com/AGamesStudios/SAGE-Engine/issues)

---

<div align="center">

Made with ❤️ by AGamesStudios

**SAGE Engine** - Build Amazing 2D Games!

</div>
