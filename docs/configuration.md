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
[window]
width = 1280
height = 720
fullscreen = false
preserve_aspect = true
allowed_resolutions = ["640x360", "960x540", "1280x720", "1600x900", "1920x1080", "2560x1440"]
[render]
scaling_mode = "fit"
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

[resource]
limit_size_mb = 10
enable_pack = true
compression = "lz4"
preload = ["sprites/", "audio/"]

[animation]
default_fps = 12
preload = ["player/", "enemies/"]


