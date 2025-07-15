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
