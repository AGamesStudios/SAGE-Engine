# UI quick start

Widgets are defined in YAML themes and emit signals on hover or click.
Use a GUI backend such as Qt or Tk to present them on screen.

```yaml
Button:
  text: Play
  on-hover: highlight_button

````python
from sage_engine import ui

button = ui.Button()
button.on_hover.connect(lambda inside: print("hover", inside))
button.hover(True)
````
```

## Themes

UI colors and fonts come from `.vel` theme files. Define hover and active
colors to highlight buttons and provide a bitmap font path for text:

```yaml
colors:
  bg: "#1e1e1e"
  fg: "#ffffff"
  accent: "#ffb400"
font:
  family: "Roboto"
  size: 14
  file: "build/font.json"
radius: 6
```

Switch the current theme at runtime:

```python
from sage_engine.ui import theme

theme.set_theme("ui/light.vel")
```

When using ``sage serve`` any change to a ``.vel`` file automatically
reloads the theme on connected clients.

## Rendering widgets

`render.draw_frame()` renders UI widgets after sprites using the
currently loaded theme. Each widget contributes a ``UIInstance`` with
position, size, color and depth so they can share the same GPU pipeline
as sprites. Widgets use ``hover`` and ``active`` colors from the theme
when the mouse is over them or a click is in progress. Call
``set_icon(path)`` on a widget to display an image loaded through the
resource manager.
