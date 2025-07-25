# Examples

The repository includes a few small demos showcasing different features of the engine.


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

## Input Test

Move a placeholder sprite with the arrow keys.

```bash
python examples/input_test/main.py
```


## FX Lab

Demonstrates Feather-FX shaders on a sprite.

```bash
python examples/fx_lab/main.py
```


## Script Watch Demo

Demonstrates the `ScriptsWatcher`. Edit `hello.py` while the demo is running to reload it.

```bash
python examples/script_watch/main.py
```


## Timer Event

Emits a `tick` event every second using a Python script.

```bash
python examples/timer_event/main.py
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

## Math Demo

Uses the math helpers to evaluate expressions and generate points.

```bash
python examples/math_demo/main.py
```


### Low Performance Mode

Append `--low-perf` to any example command to force simple rendering and
reduced effects for weak devices.
