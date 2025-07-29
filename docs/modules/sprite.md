# 📘 Модуль `sprite`

`sprite` обеспечивает базовые функции загрузки и отрисовки спрайтов формата `.sageimg`.
Он не требует GPU и использует API из `graphic`.

```python
from sage_engine.sprite import sprite, draw

spr = sprite.load("player_idle.sageimg")
draw.sprite(spr, x=10, y=20)
frame = draw.flush()
```

`draw.flush()` возвращает буфер кадра в виде `memoryview` и очищает очередь команд.
Сприты кэшируются при загрузке повторно использоваться.
