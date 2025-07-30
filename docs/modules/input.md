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
