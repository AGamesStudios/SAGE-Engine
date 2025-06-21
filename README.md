# SAGE Engine Prototype v0.0.20a

This repository demonstrates a small example using the [Ursina](https://www.ursinaengine.org/) Python engine.

The `sage_engine` package provides a lightweight wrapper around Ursina with a flexible configuration system. Settings such as window size, title, icon, asset folder and debug options are read from `sage_config.json`.
Additional options like `show_fps` and `max_fps` allow further tuning of performance. Version 0.0.20a also introduces lighting and shadow configuration so games can tweak visual quality without code changes.

Key features include:
- window and display management
- optional FPS counter and frame rate limiting
- configurable lighting with ambient color and shadows
- basic anti-aliasing and texture filtering controls

Edit this JSON file to tweak the engine without modifying Python code.

## Usage Example

Run the demo after installing Ursina:

```bash
pip install ursina
python python_examples/engine_usage.py
```

The script loads `sage_config.json` and spawns a single rotating cube. Change the
JSON values to see how the engine reacts to different window sizes and lighting
options.

### Configuration

`sage_config.json` is loaded automatically. Example values:

```json
{
  "window_size": [1536, 864],
  "title": "SAGE Engine Prototype",
  "asset_path": "python_examples",
  "debug": true,
  "show_fps": true,
  "max_fps": 120,
  "ambient_color": [0.7, 0.7, 0.8],
  "enable_shadows": true,
  "shadow_map_size": 2048,
  "anti_aliasing": true,
  "texture_filtering": "linear"
}
```

These graphics options let the engine set up basic lighting and shadow maps
automatically. `ambient_color` controls global illumination and can be tweaked
for different moods. When `enable_shadows` is true a directional light is
created with the configured `shadow_map_size`. Anti-aliasing and texture
filtering help smooth edges and textures.
