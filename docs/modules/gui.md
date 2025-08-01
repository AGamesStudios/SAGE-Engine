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

Если тема не загружена явно, `GUIManager` применяет встроенную тему по умолчанию. Она задаёт стандартный шрифт `sage_engine/resources/fonts/default.ttf`, белый цвет текста и прозрачный фон. Все новые виджеты получают эти значения автоматически. Шрифт основан на **Public Sans** и распространяется под лицензией Apache 2.0. Файл можно заменить своим, сохранив имя `default.ttf`. Если шрифт отсутствует, менеджер попробует список `fallback_fonts`, затем подключит встроенный шрифт и выведет предупреждение в лог.

В режиме отладки `manager.debug = True` поверх виджетов рисуются красные рамки, а в лог выводится содержимое текстовых элементов.

## Интернационализация
Для текстовых виджетов поддерживается загрузка словарей перевода через `gui.i18n.load()`.
Текст с префиксом `@` трактуется как ключ перевода:
```python
from sage_engine.gui import i18n, widgets
i18n.load({"main_menu.play_button": "Play"})
btn = widgets.Button(text="@main_menu.play_button")
```

\n### Visual Effects and Advanced Styling\nWidgets may use gradient backgrounds and effects. Use `gui.animation.animate` for animated properties and `gui.drag` for drag and drop events.

## Пользовательские виджеты
Вы можете определить собственный класс, наследуясь от `Widget`, и зарегистрировать его через `gui.registry`:
```python
from sage_engine.gui import base, registry

class MyBar(base.Widget):
    value: float = 0.0
    def on_draw(self, gfx):
        gfx.draw_rect(self.x, self.y, int(self.width*self.value), self.height, (0,255,0,255))

registry.register_widget("MyBar", MyBar)
```

## Привязка данных (bind)
Большинство интерактивных виджетов поддерживают `bind` для связи с объектом:
```python
slider = widgets.Slider()
slider.bind("value", settings)
```
Изменения переменной отражаются в UI и наоборот.

## Стили и переменные
Темы могут использовать переменные и условные стили:
```json
{
  "$primary": "#4444AA",
  "Button": {
    "bg_color": "$primary",
    "hover_style": {"bg_color": "#6666CC"}
  }
}
```

## Окна и панели
`Window`, `Panel` и `Popup` позволяют создавать отдельные окна и всплывающие панели. Окно можно перемещать мышью за заголовок, а popup автоматически закрывается при клике вне его области:
```python
from sage_engine.gui import widgets, manager

win = widgets.Window(title="Settings", width=120, height=80)
manager.root.add_child(win)
```

## ScrollView и инспектор
`ScrollView` отображает дочерние элементы в прокручиваемой области. `InspectorPanel` автоматически создаёт форму по атрибутам объекта и использует `bind` для изменения значений.

## Анимация виджетов
Для плавного изменения параметров используйте `gui.animation.animate`. Метод возвращает функцию отмены и может принять `on_finish`:

```python
cancel = gui.animation.animate(button, "opacity", 0, 1, 200, "ease-in", on_finish=lambda: print("done"))
```
Чтобы остановить анимацию досрочно, вызовите `cancel()`.

## Инициализация времени
Анимации используют модуль `scheduler.time`. Он автоматически инициализируется через `sage_engine.auto_setup()`. Если вы запускаете подсистемы вручную, вызовите `scheduler.time.init()` до создания анимаций.
