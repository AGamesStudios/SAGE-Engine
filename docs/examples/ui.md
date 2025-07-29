# 📘 UI cursor example

```python
from sage_engine.cursor import CursorRenderer

cursor = CursorRenderer()

# In the main loop
cursor.state.set_position(mouse.x, mouse.y)
graphic.flush()
cursor.draw()
```
