# SAGE Engine - Руководство по сборке

## Системные требования

### Минимальные требования

**Операционная система:**
- Windows 10/11 (64-bit)
- Linux (Ubuntu 20.04+, Debian 11+, Fedora 35+)
- macOS 11+ (экспериментальная поддержка)

**Аппаратное обеспечение:**
- Процессор: 2 ГГц dual-core
- Оперативная память: 2 ГБ RAM
- Графика: OpenGL 3.3+
- Свободное место: 500 МБ

### Рекомендуемые требования

**Аппаратное обеспечение:**
- Процессор: 3 ГГц quad-core или лучше
- Оперативная память: 8 ГБ RAM
- Графика: OpenGL 4.5+ (для advanced features)
- Свободное место: 2 ГБ (для сборки с debug symbols)

## Зависимости

### Обязательные библиотеки

| Библиотека | Версия | Назначение | Источник |
|------------|--------|------------|----------|
| GLFW | 3.3+ | Оконная система | Submodule |
| GLAD | OpenGL 3.3+ | OpenGL загрузчик | Submodule |
| Box2D | 3.0+ | Физический движок | Submodule |
| miniaudio | 0.11.23+ | Аудио система | Submodule |
| stb_image | 2.27+ | Загрузка изображений | Встроен |
| nlohmann/json | 3.11+ | JSON парсинг | Submodule |

### Опциональные библиотеки

| Библиотека | Версия | Назначение |
|------------|--------|------------|
| ImGui | 1.89+ | Debug UI |
| tinyxml2 | 9.0+ | XML парсинг |

### Установка зависимостей

Все обязательные зависимости включены как git submodules:

```bash
git submodule update --init --recursive
```

**Примечание:** При первом клонировании используйте:
```bash
git clone --recursive https://github.com/AGamesStudios/SAGE-Engine.git
```

---

## Инструменты разработки

### Windows

**Обязательные:**
- Visual Studio 2022 (Community или выше)
  - Desktop development with C++ workload
  - Windows 10/11 SDK
- CMake 3.15+ (можно установить через Visual Studio Installer)
- Git 2.30+

**Опциональные:**
- Ninja build system (для более быстрой сборки)
- vcpkg (для автоматического управления зависимостями)
- Visual Studio Code (с расширением CMake Tools)

**Установка Visual Studio 2022:**

1. Скачайте установщик с https://visualstudio.microsoft.com/
2. Выберите "Desktop development with C++"
3. В Individual components добавьте:
   - MSVC v143 - VS 2022 C++ x64/x86 build tools
   - Windows 10/11 SDK
   - CMake tools for Windows
4. Установите

**Установка CMake:**

```powershell
# Через Visual Studio Installer или
winget install Kitware.CMake
```

**Установка Git:**

```powershell
winget install Git.Git
```

### Linux

**Ubuntu/Debian:**

```bash
sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev
```

**Fedora:**

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    git \
    libX11-devel \
    libXrandr-devel \
    libXinerama-devel \
    libXcursor-devel \
    libXi-devel \
    mesa-libGL-devel \
    mesa-libGLU-devel
```

**Arch Linux:**

```bash
sudo pacman -S --needed \
    base-devel \
    cmake \
    git \
    libx11 \
    libxrandr \
    libxinerama \
    libxcursor \
    libxi \
    mesa
```

### macOS

**Через Homebrew:**

```bash
# Установка Homebrew (если не установлен)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Установка зависимостей
brew install cmake git
```

**Xcode Command Line Tools:**

```bash
xcode-select --install
```

## Клонирование репозитория

```bash
git clone https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
```

**Клонирование с подмодулями:**

```bash
git clone --recursive https://github.com/AGamesStudios/SAGE-Engine.git
cd SAGE-Engine
```

Если забыли --recursive:

```bash
git submodule update --init --recursive
```

## Сборка проекта

### Windows (Visual Studio)

**Генерация проекта:**

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

Опции генератора:
- `-G "Visual Studio 17 2022"` - использовать VS 2022
- `-A x64` - 64-битная сборка
- `-T host=x64` - 64-битный компилятор (быстрее)

**Сборка через CMake:**

```powershell
# Release конфигурация
cmake --build build --config Release

# Debug конфигурация
cmake --build build --config Debug

# Параллельная сборка (8 потоков)
cmake --build build --config Release -- /m:8
```

**Сборка через Visual Studio:**

1. Откройте `build/SAGE-Engine.sln`
2. Выберите конфигурацию (Debug/Release)
3. Build -> Build Solution (Ctrl+Shift+B)

**Расположение бинарных файлов:**

```
build/
  bin/
    Debug/
      SAGETests.exe
      PhysicsDemo.exe
      ...
    Release/
      SAGETests.exe
      PhysicsDemo.exe
      ...
```

### Windows (Ninja)

**Генерация:**

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

**Сборка:**

```powershell
cmake --build build
```

Ninja обычно быстрее MSVC, особенно для инкрементальных сборок.

### Linux

**Генерация:**

```bash
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
```

**Сборка:**

```bash
# Одинарный поток
cmake --build build

# Параллельная сборка (все доступные ядра)
cmake --build build -- -j$(nproc)

# Или напрямую через make
cd build
make -j$(nproc)
```

**Конфигурация Debug:**

```bash
cmake -S . -B build_debug -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build_debug -- -j$(nproc)
```

**Расположение бинарных файлов:**

```
build/
  bin/
    SAGETests
    PhysicsDemo
    ...
```

### macOS

Аналогично Linux:

```bash
cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(sysctl -n hw.ncpu)
```

## Опции CMake

### Основные опции

```bash
# Тип сборки (для single-config генераторов)
-DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel>

# Директория установки
-DCMAKE_INSTALL_PREFIX=<path>

# Компилятор C++
-DCMAKE_CXX_COMPILER=<path>

# Стандарт C++
-DCMAKE_CXX_STANDARD=17
```

### Специфичные для SAGE Engine

```bash
# Включить примеры (по умолчанию OFF)
-DBUILD_EXAMPLES=ON

# Включить тесты (по умолчанию ON)
-DBUILD_TESTS=ON

# Включить редактор (по умолчанию OFF)
-DBUILD_EDITOR=OFF

# Использовать статическую линковку (по умолчанию OFF)
-DSAGE_STATIC_LINK=ON

# Включить LTO (Link Time Optimization)
-DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON

# Включить sanitizers (для отладки)
-DSAGE_ENABLE_ASAN=ON   # Address Sanitizer
-DSAGE_ENABLE_UBSAN=ON  # Undefined Behavior Sanitizer
```

**Пример с опциями:**

```bash
cmake -S . -B build \
  -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON \
  -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
```

## Запуск тестов

### Через CTest

```bash
cd build
ctest -C Release --output-on-failure
```

Опции CTest:
- `-C <config>` - конфигурация (Debug/Release)
- `--output-on-failure` - показать вывод при ошибках
- `-V` - подробный вывод
- `-R <regex>` - запустить только тесты, соответствующие regex
- `-j <N>` - параллельный запуск

**Примеры:**

```bash
# Все тесты с подробным выводом
ctest -C Release -V

# Только физические тесты
ctest -C Release -R Physics

# Параллельный запуск (4 теста одновременно)
ctest -C Release -j 4
```

### Напрямую

**Windows:**

```powershell
cd build\bin\Release
.\SAGETests.exe
```

**Linux/macOS:**

```bash
cd build/bin
./SAGETests
```

Ожидаемый вывод:

```
[==========] Running 99 test(s)
[----------] 10 tests from ECS
[ RUN      ] ECS.EntityCreation
[  PASSED  ] ECS.EntityCreation (0.123 ms)
...
[  PASSED  ] 99 test(s)
```

## Запуск примеров

### Список примеров

После сборки с `-DBUILD_EXAMPLES=ON`:

- PhysicsDemo - демонстрация физики
- UISystemTest - UI система
- Box2DPhysicsDemo - Box2D интеграция
- AnimationDemo - система анимации
- AudioDemo - аудио система
- EventBusDemo - система событий

### Запуск

**Windows:**

```powershell
cd build\bin\Release
.\PhysicsDemo.exe
```

**Linux/macOS:**

```bash
cd build/bin
./PhysicsDemo
```

Примеры требуют наличия директории `assets/` с ресурсами.

## Установка

### Системная установка

```bash
cmake --build build --target install
```

По умолчанию устанавливается в:
- **Linux:** `/usr/local`
- **Windows:** `C:\Program Files\SAGE-Engine`
- **macOS:** `/usr/local`

Для изменения:

```bash
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/custom/path
cmake --build build --target install
```

### Компоненты установки

```
<prefix>/
  include/
    SAGE/
      Core/
      ECS/
      Graphics/
      Physics/
      Audio/
      ...
  lib/
    libSAGE.a (или SAGE.lib на Windows)
  bin/
    SAGETests
    ...
  share/
    SAGE/
      cmake/
        SAGEConfig.cmake
```

### Использование установленной библиотеки

В вашем `CMakeLists.txt`:

```cmake
find_package(SAGE REQUIRED)

add_executable(MyGame main.cpp)
target_link_libraries(MyGame SAGE::Engine)
```

## Чистая сборка

### Удаление build директории

**Windows:**

```powershell
Remove-Item -Recurse -Force build
```

**Linux/macOS:**

```bash
rm -rf build
```

### CMake clean

```bash
cmake --build build --target clean
```

Это удаляет только объектные файлы, но сохраняет конфигурацию.

## Решение проблем

### Проблема: CMake не находит компилятор

**Windows:**
```powershell
# Запустите из Developer Command Prompt for VS 2022
# или
cmake -S . -B build -G "Visual Studio 17 2022"
```

**Linux:**
```bash
sudo apt install build-essential
```

### Проблема: OpenGL не найден

**Linux:**
```bash
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

**Windows:**
OpenGL поставляется с драйверами видеокарты. Обновите драйвера.

### Проблема: Ошибки линковки с GLFW/Box2D

Убедитесь, что подмодули инициализированы:

```bash
git submodule update --init --recursive
```

Пересоберите проект:

```bash
rm -rf build
cmake -S . -B build
cmake --build build
```

### Проблема: Тесты не находят файлы ресурсов

Скопируйте директорию `assets/` в директорию с исполняемыми файлами:

**Windows:**
```powershell
Copy-Item -Recurse assets build\bin\Release\assets
```

**Linux/macOS:**
```bash
cp -r assets build/bin/
```

### Проблема: Низкая производительность Debug сборки

Это нормально. Debug сборки включают проверки и отключают оптимизацию.

Используйте Release для тестирования производительности:

```bash
cmake --build build --config Release
```

### Проблема: Ошибки компиляции в ThirdParty библиотеках

Попробуйте обновить подмодули:

```bash
git submodule update --remote --recursive
```

Или используйте конкретные версии зависимостей (см. CMakeLists.txt).

## Кастомизация сборки

### Изменение компилятора

**GCC вместо Clang:**

```bash
export CC=gcc
export CXX=g++
cmake -S . -B build
```

**Clang вместо GCC:**

```bash
export CC=clang
export CXX=clang++
cmake -S . -B build
```

**Указание конкретной версии:**

```bash
cmake -S . -B build \
  -DCMAKE_C_COMPILER=gcc-11 \
  -DCMAKE_CXX_COMPILER=g++-11
```

### Флаги компиляции

**Добавление дополнительных флагов:**

```bash
cmake -S . -B build \
  -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic"
```

**Release с debug info:**

```bash
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

### Кросс-компиляция

**Windows -> Linux (MinGW):**

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE=toolchain-mingw.cmake
```

Создайте `toolchain-mingw.cmake`:

```cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
```

## Непрерывная интеграция

### GitHub Actions

Пример `.github/workflows/build.yml`:

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, windows-latest, macos-latest]
        config: [Debug, Release]

    steps:
    - uses: actions/checkout@v3
      with:
        submodules: recursive

    - name: Configure CMake
      run: cmake -S . -B build -DCMAKE_BUILD_TYPE=${{ matrix.config }}

    - name: Build
      run: cmake --build build --config ${{ matrix.config }}

    - name: Test
      run: ctest -C ${{ matrix.config }} --output-on-failure
      working-directory: build
```

## Производительность сборки

### Ускорение сборки

**Использование Ninja:**

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

**Параллельная сборка:**

```bash
# MSVC
cmake --build build -- /m:8

# Make/Ninja
cmake --build build -- -j8
```

**Precompiled headers (автоматически в CMake 3.16+):**

В `CMakeLists.txt`:

```cmake
target_precompile_headers(SAGEEngine PRIVATE
    <vector>
    <string>
    <memory>
)
```

**ccache (Linux/macOS):**

```bash
sudo apt install ccache  # Ubuntu
brew install ccache      # macOS

export CMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake -S . -B build
```

### Время сборки

Типичное время на современном ПК (8 ядер):

- **Полная сборка Debug:** 2-3 минуты
- **Полная сборка Release:** 3-5 минут (из-за оптимизации)
- **Инкрементальная сборка:** 5-30 секунд

## Дополнительные ресурсы

- **CMake документация:** https://cmake.org/documentation/
- **Visual Studio документация:** https://docs.microsoft.com/visualstudio/
- **GCC документация:** https://gcc.gnu.org/onlinedocs/
- **Clang документация:** https://clang.llvm.org/docs/
