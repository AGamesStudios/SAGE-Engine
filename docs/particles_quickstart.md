# Particles (Ink)

`InkEmitter` is a lightweight particle system backed by a small compute buffer.
Each emitter spawns up to 2048 particles which fade over their lifetime.

```json
{
  "rate": 200,
  "velocity_range": [-20, 20],
  "life_time": 0.7,
  "color": "#ff7f00"
}
```

```python
from sage_engine.ink import InkEmitter

emitter = InkEmitter(rate=100, velocity_range=(-10, 10), life_time=0.5)
for _ in range(60):
    emitter.emit(0.016)
    emitter.update(0.016)
```

The example `ink_spark` spawns a new emitter on each mouse click using the
`spark.json` preset.
