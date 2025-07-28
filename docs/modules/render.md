# 📘 Модуль `render`

💡 Подсистема `render` отвечает за вывод изображения. Она не зависит от модуля `window` и использует выбранный backend для рисования примитивов.

## 📦 API

```python
from sage_engine import render

render.init(output_target)
render.begin_frame()
render.draw_sprite(image, x, y, w, h, rotation=0.0)
render.draw_rect(x, y, w, h, color)
render.end_frame()
render.shutdown()
```

`output_target` может быть дескриптором окна или offscreen‑буфером. Все функции делегируются выбранному backend.

## 🔹 Выбор backend

Backend выбирается через `settings.render_backend` или переменную окружения `SAGE_RENDER_BACKEND`. По умолчанию используется `software`.

Доступные реализации находятся в каталоге `render/backends`:

- `software` — простой CPU‑рендер, применяемый в тестах.
- `opengl` — заглушка для OpenGL.
- `vulkan` — будущая поддержка Vulkan.

Можно написать собственный backend, реализующий интерфейс `RenderBackend` из `render.api`.

## 🔹 Пример

```python
from sage_engine import render

render.init(None)
render.begin_frame()
render.draw_rect(10, 10, 30, 40, (1, 0, 0, 1))
render.end_frame()
render.shutdown()
```
