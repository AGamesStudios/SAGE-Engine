# 📘 Пример: интеграция Render и Window

```python
from sage_engine import window, render

window.init("Render", 320, 240)
render.init(window.get_window_handle())

while not window.should_close():
    window.poll_events()
    render.begin_frame()
    render.draw_rect(20, 20, 60, 40, (0, 255, 0))
    render.end_frame()

render.shutdown()
window.shutdown()
```
