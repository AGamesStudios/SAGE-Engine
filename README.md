# Custom Engine (SDL2 + OpenGL + Headless)

## Быстрый старт

```bash
pip install -r requirements.txt
```

### Режимы запуска
- **SDL2 + OpenGL (по умолчанию)**  
  ```bash
  python main.py --backend sdl2 --width 1280 --height 720 --srgb
  ```
- **GLFW + OpenGL**  
  ```bash
  python main.py --backend glfw
  ```
- **Headless (без графики, для CI/серверов)**  
  ```bash
  python main.py --backend headless --max-frames 300
  ```

### Горячие клавиши
- `ESC` — выход
- `F5` — wireframe toggle
- `F6` — unlit/lambert
- `F7` — сохранить сцену в `scenes/auto.json`
- `F8` — загрузить `scenes/auto.json`
- `WASD + SPACE + SHIFT` — управление камерой

### Заметки
- Для **SDL2** на Windows положите `SDL2.dll` рядом с Python или установите SDL2 через пакетный менеджер.  
- Для Linux/macOS — установите системную библиотеку SDL2 (например, `sudo apt install libsdl2-2.0-0`).
- Плагин `plugins/sage3d` регистрирует OpenGL-рендерер и автоматически добавляет **куб** в пустую сцену.
