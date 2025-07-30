# 📘 Input

Модуль `input` обрабатывает клавиатуру и мышь независимо от GUI-библиотек.
Он хранит состояние клавиш и позицию указателя, а также поддерживает простое
`action mapping`.

```python
from sage_engine.input import Input

Input.map_action("jump", key="SPACE")
Input.poll()
if Input.is_action("jump"):
    pass
```

Доступные методы:
- `poll()` — очистить временные состояния и обновить дельту мыши
- `is_pressed(key)` — удерживается ли клавиша
- `is_down(key)` — нажата ли клавиша в этом кадре
- `is_up(key)` — отпущена ли клавиша
- `get_mouse_position()` — текущая позиция мыши
- `get_mouse_delta()` — смещение мыши с прошлого кадра
- `map_action(name, key)` и `is_action(name)` — привязка действий

Допустимые клавиши: `A-Z`, `0-9`, `LEFT`, `RIGHT`, `UP`, `DOWN`, `SPACE`,
`ENTER`, `ESCAPE`, а также модификаторы `SHIFT`, `CTRL`, `ALT`.

Если в `map_action` указать неизвестное имя клавиши, в логе появится
сообщение:

```
[ERROR] [input] Unknown key: LFT (action: move)
```

При обращении к непришитому действию через `is_pressed`/`is_down`/`is_up`
выводится предупреждение:

```
[WARN] [input] Action 'move' not bound to any key
```

Сейчас полноценная поддержка реализована только для Windows через модуль
`impl.win32`. Для Linux (X11) и macOS (Cocoa) имеются заглушки, которые
позволяют собрать проект, но не обеспечивают ввод. Разработка продолжается.
