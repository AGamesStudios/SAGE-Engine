# SAGE Input

The input subsystem provides unified keyboard and mouse handling. Call
`input.update()` once per frame after polling the window to refresh the state.

```python
from sage_engine import input

# query keyboard
if input.input_key_pressed("space"):
    print("jump")
if input.input_key_down("left"):
    move_left()

# query mouse
x, y = input.input_mouse_position()
dx, dy = input.input_mouse_delta()
```

Events are emitted on state changes:
`key_down`, `key_up`, `mouse_down`, `mouse_up`, `mouse_move`, `click`.

When using the built-in window subsystem, `window.poll()` forwards events to
`input.handle_pygame_event` automatically.
