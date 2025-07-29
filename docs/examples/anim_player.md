Пример проигрывания анимации:

```python
from sage_engine.animation import AnimationPlayer
from sage_engine.sprite import sprite, draw

spr = sprite.load("enemy.sageimg")
player = AnimationPlayer("enemy_walk.sageanim", spr)
player.play()

for _ in range(10):
    player.update(100)
    player.draw(50, 50)
    draw.flush()
```
