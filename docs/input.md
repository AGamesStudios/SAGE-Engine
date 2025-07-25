# SAGE Input

The input subsystem provides unified keyboard and mouse handling. Call
`input.input_poll()` once per frame to refresh the state. The active backend is
chosen via `sage/config/input.yaml` or by calling `input.set_backend()`.

```python
from sage_engine import input

# query keyboard
if input.is_key_pressed("space"):
    print("jump")
if input.is_key_down("left"):
    move_left()

# query mouse
x, y = input.mouse_position()
dx, dy = input.mouse_delta()
```

Events are emitted on state changes:
`key_down`, `key_up`, `mouse_down`, `mouse_up`, `mouse_move`, `click`.
The default backend is a lightweight internal implementation with no external
dependencies.
