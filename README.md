# SAGE Engine Prototype v0.0.20a

This repository demonstrates a small example using the [Ursina](https://www.ursinaengine.org/) Python engine.

The `sage_engine` package provides a lightweight wrapper around Ursina with a flexible configuration system. Settings such as window size, title, icon, asset folder and debug options are read from `sage_config.json`.
Additional options like `show_fps` and `max_fps` allow further tuning of performance. Version 0.0.20a also introduces lighting and shadow configuration so games can tweak visual quality without code changes.

Key features include:
- window and display management
- optional FPS counter and frame rate limiting
- configurable lighting with ambient color and shadows
- basic anti-aliasing and texture filtering controls

Edit this JSON file to tweak the engine without modifying Python code. The example uses `ursina.prefabs.first_person_controller` for controls, so ensure Ursina is installed.

## Minecraft Style Example

A basic Minecraft-like sandbox can be launched with:

```bash
pip install ursina
python python_examples/minecraft_example.py
```

Use left-click to place a cube and right-click to remove it while hovering over an existing cube.
Scroll the mouse wheel to cycle between different block textures. A preview block in your hand shows the selected texture.

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
