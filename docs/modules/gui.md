# 📘 Модуль `gui`

`gui` предоставляет простую систему виджетов, событий и менеджер интерфейса. Она не зависит от сторонних библиотек и использует `gfx` для отрисовки примитивов.

## Основные классы
- `Widget` — базовый виджет с позицией и размерами
- `Label` — отображает текст
- `Button` — реагирует на нажатия
- `Container` — может содержать другие виджеты
- `GUIManager` — корневой контейнер и диспетчер событий

## Быстрый пример
```python
from sage_engine.graphic.manager import GUIManager
from sage_engine.graphic.widget import Button, Label

manager = GUIManager()
label = Label(text="Hello", width=80, height=20)
button = Button(text="Click", y=30, width=80, height=20)

button.on_click.connect(lambda: setattr(label, "text", "Clicked"))
manager.root.add_child(label)
manager.root.add_child(button)

# В цикле отрисовки
manager.draw()
```
