# SAGE Input

The input subsystem tracks keyboard and mouse state and emits events.

```python
from sage_engine import input

input.press_key("A")
if input.is_key_down("A"):
    print("A is held")
input.release_key("A")
```

Events dispatched:
- `key_down`, `key_up`
- `mouse_down`, `mouse_up`, `mouse_move`, `click`

When using the built-in window subsystem, calling `window.poll()` automatically
updates the input state from `pygame` events via `handle_pygame_event`.
