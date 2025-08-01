# 📘 Модуль `gui`

`gui` предоставляет независимую систему интерфейсов поверх рендера SAGE. Виджеты работают без сторонних библиотек и используют `gfx` для отрисовки.

## Основные классы
- `Widget` — базовый виджет с положением, размерами и событиями
- `GUIManager` — корневой контейнер и диспетчер событий
- `LinearLayout`, `HBoxLayout`, `VBoxLayout`, `GridLayout`, `AbsoluteLayout` — размещение дочерних элементов
- `WidgetStyle` и темы — визуальное оформление
- Стандартные виджеты: `Label`, `Button`, `TextInput`, `Checkbox`, `Slider`, `Dropdown`

## Layout & Styling

Раскладки управляют позиционированием виджетов. Пример вертикального размещения:
```python
from sage_engine.gui import widgets, layout

container = widgets.Button()
container.add_child(widgets.Button(width=50, height=20))
container.add_child(widgets.Button(width=50, height=20))
layout.VBoxLayout(spacing=4, auto_size=True).apply(container)
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
| `on_drag_start` | Начало перетаскивания |
| `on_drag_move` | Перетаскивание        |
| `on_drag_end` | Завершение перетаскивания |
| `on_drop` | Объект сброшен на цель |

## Интеграция с FlowScript
Любой обработчик событий может вызывать FlowScript-функцию. В описании виджета указывается имя функции, которая будет выполнена при событии.

## Темы
В папке `theme` находятся базовые темы: `light`, `dark`, `retro`. Вы можете подключить собственную тему через `style.load_theme()`.

## Интернационализация
Для текстовых виджетов поддерживается загрузка словарей перевода через `gui.i18n.load()`.
Текст с префиксом `@` трактуется как ключ перевода:
```python
from sage_engine.gui import i18n, widgets
i18n.load({"main_menu.play_button": "Play"})
btn = widgets.Button(text="@main_menu.play_button")
```

\n### Visual Effects and Advanced Styling\nWidgets may use gradient backgrounds and effects. Use `gui.animation.animate` for animated properties and `gui.drag` for drag and drop events.
