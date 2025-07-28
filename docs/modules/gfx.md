# 📘 Модуль `gfx`

💡 `gfx` предоставляет минимальный API для отрисовки примитивов без использования OpenGL и других графических библиотек. Рендеринг выполняется в программный буфер, который отображается в окно через нативные вызовы.

## 📦 API

```python
from sage_engine import gfx

gfx.init(window_handle)
gfx.begin_frame()
gfx.draw_rect(x, y, w, h, (r, g, b))
gfx.end_frame()
gfx.shutdown()
```

## 🔹 Реализация

Сейчас поддерживается только программный бэкенд для Windows. Все пиксели записываются в `bytearray`, а затем выводятся через GDI. На других платформах функции работают как заглушки и не бросают исключения.

## 🔹 Пример

```python
from sage_engine import window, gfx

window.init("Test", 640, 480)
gfx.init(window.get_window_handle())
while not window.should_close():
    window.poll_events()
    gfx.begin_frame()
    gfx.draw_rect(10, 10, 30, 30, (0, 255, 0))
    gfx.end_frame()

gfx.shutdown()
window.shutdown()
```
