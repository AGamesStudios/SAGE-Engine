# 📘 Пример: базовая сцена

```python
from sage_engine import world, window, render

window.init("Example", 1280, 720)
render.init(window.get_window_handle())
render.set_viewport(1280, 720, preserve_aspect=True)

edit = world.scene.begin_edit()
player = edit.create(role="sprite", name="Player", x=0, y=0)
world.scene.apply(edit)
world.scene.commit()
```

При запуске на экране 1920×1080 изображение будет масштабировано до 1280×720 без
искажений и центрировано.

