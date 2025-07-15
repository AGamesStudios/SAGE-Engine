# Fonts and text

Bitmap fonts are loaded from BMFont ``.json`` or ``.fnt`` files alongside a PNG atlas.
Use ``sage_engine.render.font.load`` to create a ``Font`` object, then pass it to
``TextObject`` to display a string.

```python
from sage_engine.render.font import load
from sage_engine import text, render

font = load("myfont.json")
hello = text.TextObject("Hello", 0, 0, font)
text.add(hello)
backend = render.get_backend("headless")
backend.create_device(200, 100)
render.draw_frame(backend)
```

UI widgets like ``Label`` and ``Button`` can receive a ``Font`` instance and will
render their text using the same sprite pass as regular sprites.
