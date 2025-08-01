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

Для массовой отрисовки используйте `SpriteBatch`:

```python
from sage_engine.sprite import sprite_batch, draw

batch = sprite_batch.SpriteBatch()
batch.add(spr, 10, 20, 32, 32)
frame = draw.draw_batch(batch)
```

Спрайты из атласа доступны через `atlas.load_from_texture_atlas()` и
`draw.sprite_from_atlas(name, x, y)`.
