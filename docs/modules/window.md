# 📘 Модуль `window`

💡 Подсистема `window` отвечает за создание и управление окном приложения. Она не
использует сторонние библиотеки и по умолчанию полагается на `tkinter`. Если зап
устить движок в режиме `headless`, используется внутренняя заглушка.

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

Backend вынесен в модуль `window.impl.tk`, что упрощает добавление альтернативных реализаций.
