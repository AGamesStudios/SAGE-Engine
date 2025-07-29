# 📘 Module `fsync`

`fsync` provides platform independent frame synchronization. It targets a stable
frame rate without relying on VSync or GPU timing.

## Usage
```python
from sage_engine.runtime.fsync import FrameSync

fs = FrameSync(target_fps=60)
while running:
    fs.start_frame()
    update()
    draw()
    graphic.flush()
    fs.end_frame()
```

`mode` controls behavior:
- `precise` – keeps the exact target fps
- `adaptive` – drops to a factor of the target if drawing is too slow
- `smooth` – slowly adjusts to the actual frame duration

`correct_phase` happens automatically in `start_frame`/`end_frame` when drift
exceeds the configured tolerance.
