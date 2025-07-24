# Examples

The repository includes a few small demos showcasing different features of the engine.

## Alpha Demo

Demonstrates the low-level ChronoPatch API and MicroPython bindings. Run it with:

```bash
python examples/alpha_demo/main.py
```

## Example Project

A minimal game loop written in Python. It opens a window, loads objects and executes a Lua script that moves the `player` sprite based on keyboard input.

```bash
python examples/example_project/main.py
```

## Basic Display

Shows a UI label and camera using FlowScript.

```bash
python examples/basic_display/main.py
```

## Object Control

Move a sprite with arrow keys via Lua.

```bash
python examples/object_control/main.py
```

## Multi Script

Loads FlowScript, Lua and Python at once.

```bash
python examples/multi_script/main.py
```

## Camera Follow

Camera follows the moving sprite using Python.

```bash
python examples/camera_follow/main.py
```

## Event Button

Press SPACE to emit a custom event.

```bash
python examples/event_button/main.py
```

## FX Lab

Demonstrates Feather-FX shaders on a sprite.

```bash
python examples/fx_lab/main.py
```

## Clear Color Demo

Shows how to change the background color.

```bash
python examples/clear_color_demo/main.py
```

## Script Watch Demo

Demonstrates the `ScriptsWatcher`. Edit `hello.py` while the demo is running to reload it.

```bash
python examples/script_watch/main.py
```

## Profile Demo

Prints initialization timings for each subsystem.

```bash
python examples/profile_demo/main.py
```

## Timer Event

Emits a `tick` event every second using a Python script.

```bash
python examples/timer_event/main.py
```

## FrameSync Demo

Displays the measured FPS.

```bash
python examples/framesync_demo/main.py
```

## Python Spawn

Creates a few sprite objects from a Python script.

```bash
python examples/python_spawn/main.py
```

## Python Globals

Shows how to inject custom globals for Python scripts.

```bash
python examples/python_globals/main.py
```
