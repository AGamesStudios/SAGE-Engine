# 📘 Пример: интеграция Render и Window

```python
from sage_engine import window, gfx

window.init("Render", 320, 240)
gfx.init(window.get_window_handle())

while not window.should_close():
    window.poll_events()
    gfx.begin_frame()
    gfx.draw_rect(20, 20, 60, 40, (0, 255, 0))
    gfx.end_frame()

gfx.shutdown()
window.shutdown()
```
