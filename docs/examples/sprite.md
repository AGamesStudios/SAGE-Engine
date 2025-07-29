# 📘 Пример использования спрайтов

```python
from sage_engine.sprite import sprite, draw
from sage_engine.graphic import api

api.init(64, 64)
player = sprite.load("player_idle.sageimg")
draw.sprite(player, x=32, y=32)
image = draw.flush()
```

Функция `draw.flush()` применяет отложенные команды и возвращает содержимое кадра.
