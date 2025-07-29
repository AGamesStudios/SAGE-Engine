# 📘 Пример: базовая сцена

```python
from sage_engine import world, window, render, gfx

window.init("Example", 1280, 720)
render.init(window.get_window_handle())
render.set_viewport(1280, 720, preserve_aspect=True)

edit = world.scene.begin_edit()
player = edit.create(role="sprite", name="Player", x=0, y=0)
world.scene.apply(edit)
world.scene.commit()

gfx.begin_frame()
world.scene.render()
buf = gfx.end_frame()
render.present(buf)
```

При запуске на экране 1920×1080 изображение будет масштабировано до 1280×720 без
искажений и центрировано.

