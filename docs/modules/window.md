# 📘 Модуль `window`

💡 Подсистема `window` отвечает за создание и управление окном приложения. Она не
использует сторонние GUI-библиотеки и по умолчанию выбирает нативный backend
для текущей платформы. Если запустить движок в режиме `headless`, используется
внутренняя заглушка.

## 📦 API

```python
init(title: str, width: int, height: int, fullscreen: bool = False,
     resizable: bool = True, borderless: bool = False)
```
Создаёт окно с указанными параметрами.

```python
poll_events()
```
Обрабатывает системные события. Необходимо вызывать каждый кадр.

```python
get_size() -> tuple[int, int]
```
Возвращает текущий размер окна.

```python
should_close() -> bool
```
Флаг закрытия окна.

```python
shutdown()
```
Закрывает окно и освобождает ресурсы.

```python
get_window_handle() -> Any
```
Возвращает нативный дескриптор окна для передачи в рендер.

## 🔧 События

При закрытии и ресайзе окна автоматически испускаются события через модуль
`events`:

- `WIN_CLOSE` – идентификатор события закрытия.
- `WIN_RESIZE` – изменение размеров, аргументы `(width, height)`.
- `WIN_KEY` – нажатие клавиши, аргументы `(keysym, keycode)`.
- `WIN_MOUSE` – движение или нажатие мыши, `(type, x, y, button)`.

Используйте `events.dispatcher.on(WIN_CLOSE, handler)` чтобы реагировать на них.

## 🔹 Пример

```python
from sage_engine import window

window.init("My Game", 800, 600)
while not window.should_close():
    window.poll_events()
    # обновление, рендер
window.shutdown()
```


## 💡 Рекомендации

`poll_events()` уже включает небольшую задержку, поэтому постоянный вызов этой функции в цикле не создаёт заметной нагрузки на процессор. Если окно неактивно, использование `SAGE_HEADLESS=1` позволит полностью отключить графику.

Backend выбирается автоматически из каталога `window.impl` (`win32`, `x11` или `cocoa`), что упрощает добавление новых платформенных реализаций.

### Windows Notes

На Windows поддерживаются версии Python 3.10–3.12. Перед запуском убедитесь,
что установлены необходимые Visual C++ библиотеки. Переменная окружения
`SAGE_LOGLEVEL` позволяет переключить уровень логов (`DEBUG`, `INFO`, `WARN`, `ERROR`).

## 🔹 Platform-specific backends

В каталоге `window/impl` находятся реализации для разных систем:

- `win32` — нативный Win32 API через `ctypes`.
- `x11` — работа с X11 на Linux.
- `cocoa` — использование Cocoa на macOS.

Функция `window.init` автоматически выбирает нужный класс по `sys.platform`. При
отсутствии подходящего backend можно установить переменную окружения
`SAGE_HEADLESS=1` чтобы использовать внутренний headless‑режим.

## 🔹 Win32 backend

При создании окна на Windows через `ctypes` после `CreateWindowEx` необходимо
вызвать `ShowWindow` и `UpdateWindow`, иначе окно может не отобразиться. Цикл
`poll_events()` выполняет обработку сообщений через `PeekMessage`/`DispatchMessage`
и содержит небольшую задержку, чтобы не загружать CPU. Обработчик окна
генерирует события `WIN_CLOSE` и `WIN_RESIZE` при получении `WM_CLOSE` и
`WM_SIZE` соответственно.

### Проверка Win32 окна

1. Запустите `examples/demo_game/demo.py` на Windows.
2. Измените размеры и перемещайте окно – в консоль будут печататься события
   `WIN_RESIZE` и `WIN_MOUSE`.
3. Проверьте флаги `resizable`, `borderless` и `fullscreen` при инициализации.
   В режимах `borderless` и `fullscreen` рамка окна исчезает, а перетаскивание
   работает только по верхней полосе.
