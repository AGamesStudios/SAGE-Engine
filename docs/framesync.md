# FrameSync

The FrameSync subsystem regulates the update rate without relying on GPU VSync.
It uses a small sleep-and-spin strategy to hit a target frame time while keeping
input latency low.

Configuration lives in `sage/config/framesync.yaml`:

```yaml
framesync:
  enabled: true
  target_fps: 60
  allow_drift: false
  profile: balanced
```

Call `framesync.regulate()` once per frame to delay until the next frame
boundary. When `allow_drift` is false the regulator keeps the exact phase over
time. Enabling drift lets it adapt if frame times vary greatly.
