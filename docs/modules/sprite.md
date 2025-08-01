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

Для совместного управления несколькими спрайтами используйте `SpriteGroup`:

```python
from sage_engine.sprite import group, sprite_batch, draw

grp = group.SpriteGroup(x=100, y=100, scale=1.5)
grp.add(spr)
batch = sprite_batch.SpriteBatch()
grp.draw(batch)
draw.draw_batch(batch)
```

Спрайты из атласа доступны через `atlas.load_from_texture_atlas()` и
`draw.sprite_from_atlas(name, x, y)`.

Для текста используется `sprite.text`:
```python
from sage_engine.sprite import text

font = text.load_font("sage_engine/resources/fonts/default.ttf", 14)
text.draw_text("Hello", 40, 20, font)
```

Функции `sprite.draw` и `sprite.text.draw_text` можно вызывать в одном кадре,
объединяя спрайты и текст. Все используемые текстуры поступают из
`TextureCache`, включая глифы шрифта по умолчанию `default.ttf`.
