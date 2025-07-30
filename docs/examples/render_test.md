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

Approximate frame times on a mid-range CPU:

| Objects | Frame Time |
|---------|------------|
| 10k     | ~6 ms |
| 100k    | ~12 ms |
| 500k    | ~30 ms |

Chunking, culling and batching can be toggled via the `[render]` section in
`game.sagecfg` to experiment with different optimization levels.
