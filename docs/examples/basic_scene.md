# 📘 Пример: базовая сцена

```python
from sage_engine import world

edit = world.scene.begin_edit()
player = edit.create(role="sprite", name="Player", x=0, y=0)
world.scene.apply(edit)
world.scene.commit()
```
