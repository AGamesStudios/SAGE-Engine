# SAGE Engine

This repository demonstrates a small example using the [Ursina](https://www.ursinaengine.org/) Python engine.

The `sage_engine` package provides a lightweight wrapper around Ursina with a simple configuration system. Settings such as window size, title, and icon are read from `sage_config.json`.

Edit this JSON file to tweak window size, title, or icon without modifying code. The example uses `ursina.prefabs.first_person_controller` for controls, so ensure Ursina is installed.

## Minecraft Style Example

A basic Minecraft-like sandbox can be launched with:

```bash
pip install ursina
python python_examples/minecraft_example.py
```

Use left-click to place a cube and right-click to remove it while hovering over an existing cube.
