# 📘 Модуль `gfx`

💡 `gfx` предоставляет минимальный API для отрисовки примитивов без использования OpenGL и других графических библиотек. Рендеринг выполняется в программный буфер, который отображается в окно через нативные вызовы.

## 📦 API

```python
from sage_engine import gfx, render

gfx.init(width, height)
gfx.begin_frame()
gfx.draw_rect(x, y, w, h, (r, g, b, a))
gfx.add_effect("blur")
buffer = gfx.end_frame()
render.present(buffer)
gfx.shutdown()
```

## 🔹 Реализация

Сейчас поддерживается только программный бэкенд для Windows. Все пиксели записываются в `bytearray`, а затем выводятся через GDI. На других платформах функции работают как заглушки и не бросают исключения.

Поддерживается альфа‑канал и простые эффекты. Встроенный фильтр `blur` демонстрирует обработку всего кадра. Цвет может задаваться кортежем `(r, g, b, a)` или строкой `"#RRGGBBAA"`.

Для сложных сцен создайте `gfx.Scene` и несколько `gfx.Layer` с различными `z`‑значениями.

## 🔹 Пример

```python
from sage_engine import window, render, gfx

window.init("Test", 640, 480)
gfx.init(640, 480)
render.init(window.get_window_handle())
while not window.should_close():
    window.poll_events()
    gfx.begin_frame()
    gfx.draw_rect(10, 10, 30, 30, "#00FF0080")
    buffer = gfx.end_frame()
    render.present(buffer)

gfx.shutdown()
render.shutdown()
window.shutdown()
```
