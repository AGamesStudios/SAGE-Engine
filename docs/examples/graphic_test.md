# 🔹 Пример работы с `gfx` слоями и эффектами

```python
from sage_engine import window, render, gfx
from sage_engine.graphic.scene import Scene, Layer

window.init("Graphic", 320, 240)
gfx.init(320, 240)
render.init(window.get_window_handle())

scene = Scene()
base = Layer(z=0)
top = Layer(z=5)
scene.add(base)
scene.add(top)

scene.rect(base, 20, 20, 80, 40, "#00FF00")
scene.rect(top, 40, 40, 80, 40, (255, 0, 0, 128))

gfx.add_effect("blur")
while not window.should_close():
    window.poll_events()
    gfx.begin_frame()
    scene.render()
    buffer = gfx.end_frame()
    render.present(buffer)

gfx.shutdown()
render.shutdown()
window.shutdown()
```
