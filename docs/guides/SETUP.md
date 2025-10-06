# Инструкция по настройке SAGE Engine

## Шаг 1: Установка зависимостей

### Windows

1. **Установите CMake**
   - Скачайте с https://cmake.org/download/
   - Добавьте в PATH

2. **Установите компилятор C++**
   - Visual Studio 2019 или новее (рекомендуется)
   - Или MinGW-w64

### Шаг 2: Загрузка сторонних библиотек

Выполните следующие команды в корне проекта:

```powershell
# Создание директорий для сторонних библиотек
New-Item -ItemType Directory -Force -Path "ThirdParty"

# GLFW
git clone --depth 1 --branch 3.3.8 https://github.com/glfw/glfw.git ThirdParty/glfw

# GLAD
New-Item -ItemType Directory -Force -Path "ThirdParty/glad/include/glad"
New-Item -ItemType Directory -Force -Path "ThirdParty/glad/include/KHR"
New-Item -ItemType Directory -Force -Path "ThirdParty/glad/src"
```

Затем скачайте GLAD:
1. Перейдите на https://glad.dav1d.de/
2. Выберите:
   - Language: C/C++
   - Specification: OpenGL
   - API gl: Version 3.3 (или выше)
   - Profile: Core
3. Нажмите GENERATE
4. Скачайте ZIP
5. Распакуйте содержимое в `ThirdParty/glad/`

### Шаг 3: Сборка проекта

```powershell
# Создание директории для сборки
mkdir build
cd build

# Генерация проекта
cmake ..

# Сборка
cmake --build . --config Release

# Запуск Sandbox
.\Sandbox\Release\Sandbox.exe
```

## Альтернативный метод: Использование vcpkg

```powershell
# Установка vcpkg
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Установка зависимостей
.\vcpkg\vcpkg install glfw3:x64-windows

# Сборка с vcpkg
cmake .. -DCMAKE_TOOLCHAIN_FILE=[путь к vcpkg]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Структура после установки

```
SAGE-Engine/
├── ThirdParty/
│   ├── glfw/
│   └── glad/
│       ├── include/
│       │   ├── glad/
│       │   └── KHR/
│       └── src/
├── Engine/
├── Sandbox/
└── build/
```

## Пользовательские шрифты и проверка UI

### Подключение собственных TTF/OTF

1. Скопируйте файлы шрифтов в каталог `Demo/assets/fonts`.
2. Для кастомных наборов создайте подпапку `Demo/assets/fonts/custom` — движок просканирует её рекурсивно.
3. Дополнительные каталоги можно указать через переменную окружения `SAGE_FONT_DIRS` (для Windows разделитель `;`). Пример:

   ```powershell
   $env:SAGE_FONT_DIRS = "D:\\Fonts;C:\\Assets\\UI\\Fonts"
   ```

4. Отдельные файлы можно перечислить в `SAGE_FONT_FILES` (те же разделители). Движок автоматически регистрирует только форматы `.ttf` и `.otf`. Если загрузка кастомного шрифта не удалась, система откатится к системному или встроенному ProggyClean.

### Проверка UI и ресурсов

После изменений сборки рекомендуется запустить smoke-тесты:

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug -R SAGE_Engine_Tests
```

Тесты проверяют корректность перетаскивания панелей, регистрацию аудио/шрифтов и общую устойчивость UI.
