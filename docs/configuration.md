# 📘 Configuration

`engine.sagecfg` stores engine options. The `[graphics]` section controls the rendering pipeline.

Example:

```ini
[graphics]
antialiasing = "SmartSubpixel8x"
filtering = "bicubic"
dynamic_resolution = true
gamma_correction = true
style = "neo-retro"
fallback_mode = "EdgeAverage2x"
[fsync]
enabled = true
target_fps = 60
mode = "precise"
tolerance = 0.2
```

[cursor]
enabled = true
style = "default"
hide_system_cursor = true
follow_rate = 1.0

