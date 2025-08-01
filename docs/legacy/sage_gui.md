# 📘 SAGE GUI

`SAGE GUI` \- собственная библиотека интерфейсов для движка SAGE. Она заменяет `tkinter` и предоставляет виджеты, систему событий и рендер через `SAGE Graphic`.

## 🔹 Особенности
- Независимость от сторонних GUI‑фреймворков
- Отрисовка через внутренний `gfx` модуль
- Поддержка тем и стилей (`theme/dark.sagegui`)
- Контейнеры `VBox`, `HBox` для компоновки

## 🔹 Использование
```python
from sage_gui import GUIContext, Button, VBox

ctx = GUIContext()
root = VBox()
btn = Button(text="Run")
root.add_child(btn)
ctx.add_widget(root)
ctx.update(0.016)
ctx.draw()
```
