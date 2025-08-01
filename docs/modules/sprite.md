# 📘 Модуль `sprite`

`sprite` обеспечивает классы `Sprite` и функции отрисовки. Система использует только формат `.sageimg` и работает поверх `gfx` без прямого доступа к GPU.

```python
from sage_engine.sprite import sprite, draw

spr = sprite.load("player_idle.sageimg")
draw.sprite(spr, x=10, y=20)
frame = draw.flush()
```

`draw.flush()` возвращает буфер кадра в виде `memoryview` и очищает очередь команд.
Сприты кэшируются при загрузке повторно использоваться.
