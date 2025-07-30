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
render.present(buffer)
render.shutdown()
```

```python
render.set_viewport(1280, 720, preserve_aspect=True)
```
Устанавливает логическое разрешение окна с сохранением пропорций.

`output_target` может быть дескриптором окна или offscreen‑буфером. Все функции делегируются выбранному backend.

## 🔹 Выбор backend

Backend выбирается через `settings.render_backend` или переменную окружения `SAGE_RENDER_BACKEND`. По умолчанию используется `software`.

Доступные реализации находятся в каталоге `render/backends`:

- `software` — простой CPU‑рендер, применяемый в тестах.
- `vulkan` — будущая поддержка Vulkan.

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

## 🔹 Формат буфера

Буфер кадра всегда имеет формат **BGRA8 premultiplied**. Размер строки равен `width * 4` байта. Это совпадает с форматом `CreateDIBSection` на Windows, поэтому копирование выполняется без конверсий.

## 🔹 Жизненный цикл

1. `init(handle)` – создание контекста.
2. `begin_frame()` – подготовка.
3. `draw_*()` – накопление команд.
4. `end_frame()` – финализация.
5. `present(buffer)` – копирование и вывод.
6. `resize(w, h, handle)` – пересоздание буфера при изменении размеров.
7. `shutdown()` – освобождение всех ресурсов.

Можно создать несколько контекстов через `create_context(handle)` и передавать нужный `handle` в `present()`.

## 🔹 Производительность

Стандартный путь использует `memcpy` из кадра в память DIB и далее `BitBlt` в окно. При необходимости можно получить прямой доступ к памяти окна через `render._get_backend()._contexts[handle].bits` и рисовать без копии.

## Автосогласование графических модулей

`render` получает буфер из `gfx.flush_frame()`. Перед выводом размер буфера
проверяется и сопоставляется с текущим контекстом. Если размеры не совпадают,
в журнал пишется ошибка и кадр игнорируется, что предотвращает падение
движка при ошибочных параметрах.

## Автоматическая адаптация буфера к размеру окна

При вызове `gfx.flush_frame()` размер кадрового буфера сравнивается с
текущими параметрами окна. Если `gfx._runtime.auto_resize` включён, `gfx`
пересоздаёт буфер и затем передаёт его в `render.present`. Это устраняет
ошибку «Buffer size too small» при динамическом изменении размеров окна.

## Ошибки и диагностика

`render.present` сверяет размер переданного буфера с размером контекста.
Если буфер меньше ожидаемого, в журнал пишется сообщение уровня ERROR и
поднимается `ValueError`. Такое поведение предотвращает вывод случайных
данных и помогает выявить несогласованность между `gfx` и `render`.
Рекомендуется вызывать `gfx.flush_frame()` вместо прямого `render.present`,
чтобы буфер автоматически пересоздавался при изменении размеров окна.


## Расширенные возможности

`render` поддерживает набор оптимизаций: Predictive Micro-Culling, Delta-Render и Raster Cache Manager. Они включаются через функции `enable_micro_culling()`, `enable_delta_render()` и `enable_raster_cache()`. Адаптивный планировщик перерисовки (`enable_adaptive_repaint()`) снижает нагрузку, когда кадры выходят за заданный бюджет.

Для ограничения времени кадра используется `set_frame_budget(ms)`. Если время между `begin_frame()` и `present()` превышает бюджет, в журнале появляется предупреждение.

Диаграмма конвейера отображена в файле `docs/diagrams/render_pipeline.svg`.
