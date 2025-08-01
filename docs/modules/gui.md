# 📘 Модуль `gui`

`gui` предоставляет независимую систему интерфейсов поверх рендера SAGE. Виджеты работают без сторонних библиотек и используют `gfx` для отрисовки.

## Основные классы
- `Widget` — базовый виджет с положением, размерами и событиями
- `GUIManager` — корневой контейнер и диспетчер событий
- `LinearLayout`, `GridLayout`, `AbsoluteLayout` — размещение дочерних элементов
- `WidgetStyle` и темы — визуальное оформление
- Стандартные виджеты: `Label`, `Button`, `TextInput`, `Checkbox`, `Slider`, `Dropdown`

## Layout & Styling

Раскладки управляют позиционированием виджетов. Пример вертикального размещения:
```python
from sage_engine.gui import manager, widgets, layout

container = widgets.Button(width=100, height=20)
layout.LinearLayout().apply(container)
```
Темы загружаются из JSON и могут переключаться в рантайме:
```python
from sage_engine.gui import style
style.load_theme("dark", "path/to/dark.json")
style.apply_theme(button.style, "dark")
```

## События
| Событие     | Описание               |
|-------------|-----------------------|
| `on_click`  | Клик мыши по виджету   |
| `on_hover`  | Наведение курсора      |
| `on_focus`  | Получение фокуса       |
| `on_change` | Изменение значения     |
| `on_keypress` | Ввод символа         |

## Интеграция с FlowScript
Любой обработчик событий может вызывать FlowScript-функцию. В описании виджета указывается имя функции, которая будет выполнена при событии.

## Темы
В папке `theme` находятся базовые темы: `light`, `dark`, `retro`. Вы можете подключить собственную тему через `style.load_theme()`.

