# Render Test

This example spawns many rectangles to stress-test the rendering pipeline.
The number of objects and window size are configured via `game.sagecfg`.

Run it with:

```bash
python examples/render_test/main.py
```

Example configuration:

```cfg
name = "Render Test"
width = 640
height = 360
count = 10000
```
