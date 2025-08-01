# 📈 High-performance rendering

Этот документ описывает приёмы оптимизации SAGE Engine на чистом Python.

- Используйте `sprite_batch.SpriteBatch` для накопления спрайтов без списков.
- Координаты и размеры хранятся в массивах `array('f')`, цвета — `array('B')`.
- Функции из `render.mathops` работают с фиксированной точкой Q8.8.
- Все цвета конвертируются в premultiplied RGBA при загрузке.

Пример:

```python
from sage_engine.sprite import sprite_batch, draw

batch = sprite_batch.SpriteBatch(10000)
# fill batch in update()
image = sprite.load("enemy.sageimg")
for i in range(10000):
    batch.add(image, i, 0, image.width, image.height, (255, 255, 255, 255))

frame = draw.draw_batch(batch)
```
