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
- `opengl` — реализация на Win32 через `ctypes`.
- `vulkan` — будущая поддержка Vulkan.

### OpenGL backend

Бэкенд `opengl` работает только на Windows и использует `ctypes` для вызовов
`wglCreateContext`, `wglMakeCurrent` и других функций из `opengl32.dll`. Для
вывода кадра вызывается `SwapBuffers`. Этот бэкенд предназначен для простого
рендеринга прямоугольников и спрайтов в окне, полученном из `window`.

Можно написать собственный backend, реализующий интерфейс `RenderBackend` из `render.api`.

## 🔹 Пример

```python
from sage_engine import render, window

window.init("Example", 640, 480)
render.init(window.get_window_handle())
render.begin_frame()
render.draw_rect(10, 10, 30, 40, (1, 0, 0, 1))
render.end_frame()
render.shutdown()
window.shutdown()
```
