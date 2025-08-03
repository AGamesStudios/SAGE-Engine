# 📘 Модуль `graphic`

`graphic` v2 строится на программном рендере и не требует OpenGL или DirectX.
Через API [`graphic.api`](../../sage_engine/graphic/api.py) можно рисовать
спрайты и линии, а `flush()` применяет эффекты и стили к буферу.
Все координаты передаются в экранном пространстве; для перевода мировых
координат используйте функции модуля `transform`.

Основные компоненты:
- `api` — единая точка вызова (`draw_sprite`, `draw_line`, `flush`)
- `fx` — эффекты сглаживания
- `style` — пост‑фильтры (`mono-dark`, `neo-retro` и др.)
- `backend` — низкоуровневая буферизация
- `state` — текущие настройки и конфигурация

## 🔹 Совместимость

Все сохранения автоматически мигрируются функцией `compat.convert()` без
использования номеров.

## 📦 Основные классы

- `Layer(z)` — контейнер объектов на одном глубинном уровне.
- `Group(state=None)` — набор примитивов со своим состоянием. При рендере
  состояние применяется стеково.
- `Scene` — хранит набор слоев и выводит их через `render()`.

## 🔧 Стили

Используйте `gfx.push_state()` и `gfx.pop_state()` для временного изменения
`gfx.state`. Состояние можно сохранить через `snapshot()` и восстановить через
`restore()`.

## 🧠 Состояние рендера

Для глобального управления параметрами рендера используйте функции:

```python
from sage_engine.graphic import state

state.set_state("style", "neo-retro")
current = state.get_state("style")
state.clear_state()
cfg = state.export_state()
```

`set_state(key, value)` задаёт параметр, `get_state(key)` возвращает значение,
`clear_state()` сбрасывает все параметры. `export_state()` выдаёт копию текущего
словаря состояний.

## 🔹 Примитивы

`gfx` поддерживает функции `draw_rect`, `draw_circle`, `draw_line`,
`draw_polygon`, `draw_rounded_rect` и заглушку `draw_text`.

По умолчанию текст выводится шрифтом `sage_engine/resources/fonts/default.ttf`.
Это файл **Public Sans** под Apache 2.0. При загрузке через
`sprite.text.load_font()` глифы помещаются в текстуру и кэшируются. Вызов
`draw_text` использует эту текстуру без обращения к файлу `.ttf`.

Статистика отрисовки доступна через `render.stats`. На каждый кадр обнуляются
`sprites_drawn`, `text_glyphs_rendered`, `textures_loaded`, `atlas_hits`,
`atlas_misses` и `textures_bound`. Поле `texture_memory_kb` показывает текущий
объём памяти текстур, `memory_peak` фиксирует максимум за сессию, а
`time_spent_ms` — длительность последнего кадра.
В любой момент данные можно вывести через `sage debug stats`.

## ✨ Эффекты

Эффекты регистрируются функцией `graphic.fx.register(name, func)` и могут быть
применены через `gfx.add_effect`.

## 🔹 Pixel format

Все операции выполняются в формате **BGRA8 premultiplied**. Входные цвета
приводятся к этому формату функцией `to_bgra8_premul`. Формулы блендинга:

```
out.rgb = src.rgb + dst.rgb * (1 - src.a)
out.a   = src.a   + dst.a   * (1 - src.a)
```

## 🔹 Очистка кадра

`gfx.begin_frame((0,0,0,255))` всегда заполняет весь буфер заданным
цветом. Следуйте порядку: `begin_frame` → `scene.render()` → прямые вызовы
`gfx.draw_*` → `end_frame`.

## 🔹 Z и порядок

Команды сортируются по `(z, seq)` и выполняются стабильно. Объекты с одинаковым
`z` рисуются в порядке добавления.

## 🔹 Альфа-политика

Все цвета считаются premultiplied. Полезно заранее умножать `r`, `g`, `b` на
`a/255` при вычислении цветов.

## 🔹 GUI
Модуль содержит базовые виджеты, систему раскладок и менеджер интерфейса для GUI.
Подробнее см. [gui.md](gui.md).
\n## Visual Effects and Advanced Styling\nUse `graphic.effects` and `graphic.gradient` to apply gradients and post effects. `graphic.animation` provides property animations for widgets.
