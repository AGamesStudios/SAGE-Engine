# 📘 Модуль `animation`

`animation` воспроизводит покадровые файлы `.sageanim` и тесно интегрирован с `sprite`.

```python
from sage_engine.animation import AnimationPlayer
from sage_engine.sprite import sprite

spr = sprite.load("player_idle.sageimg")
player = AnimationPlayer("player_walk.sageanim", spr)
player.play()
player.update(16)
player.draw(0, 0)
```

Методы `play`, `pause`, `stop`, `set_loop` и `goto_frame` позволяют управлять
анимацией. `update(dt)` принимает время в миллисекундах.
