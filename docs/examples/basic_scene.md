# 📘 Пример: базовая сцена

```python
from sage_engine import scene

edit = scene.scene.begin_edit()
player = edit.create(role="sprite", name="Player", x=0, y=0)
scene.scene.apply(edit)
scene.scene.commit()
```
