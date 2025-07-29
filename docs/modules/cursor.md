# 📘 Cursor

Модуль `cursor` рисует собственный спрайт поверх сцены и не зависит от системного курсора.

```python
from sage_engine.cursor import CursorRenderer

cursor = CursorRenderer()
cursor.state.set_position(10, 10)
cursor.state.set_visible(True)
```

`CursorState.follow_rate` управляет плавностью движения. Стиль загружается из `.sagecurs` и может быть изменён во время игры.
