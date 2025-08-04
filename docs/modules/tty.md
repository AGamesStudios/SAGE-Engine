# TTY Module

The **TTY** subsystem enables running the engine entirely in a text-only
terminal. It provides a fixed 80×25 character buffer, simple drawing
primitives and non-blocking keyboard input.

## Buffer

`TTYBuffer` stores a 2D grid of `TTYCell` objects. Each cell tracks the
character, foreground color, background color and bold style.

## Drawing

High level helpers such as `draw_text` and `draw_rect` modify the buffer.
During the draw phase the buffer is written to the terminal using ANSI
escape sequences.

## Input

`tty.input` maps key presses to actions. The state is refreshed every
update and can be queried with `is_pressed(name)`.

## Integration

Importing `sage_engine.tty` registers the subsystem with the core
registry. An application can then render text with:

```python
from sage_engine import tty

TTY = core.get("tty")
tty.draw_text(10, 2, "Welcome to SAGE!", fg="cyan")
```

The engine switches into TTY mode when `engine.sagecfg` contains:

```ini
[run]
mode = tty
```
