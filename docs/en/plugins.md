# Plugin Registration

SAGE Engine can be extended at runtime with your own object types, input backends and renderers.

## Objects

Use `engine.core.objects.register_object` as a decorator:

```python
from engine.core.objects import register_object

@register_object('my_object', [('name', None)])
class MyObject:
    def __init__(self, name='MyObject'):
        self.name = name
        self.role = 'my_object'
        self.group = None  # optional scene grouping
```

Objects may set ``group`` to categorize them. The scene helper
``Scene.iter_group('enemies')`` yields all objects assigned to a group.

## Input Backends

Create a class implementing `InputBackend` and register it:

```python
from engine.inputs import InputBackend, register_input

class FancyInput(InputBackend):
    def poll(self):
        pass
    def is_key_down(self, key: int) -> bool:
        return False
    def is_button_down(self, btn: int) -> bool:
        return False
    def shutdown(self):
        pass

register_input('fancy', FancyInput)
```

Built-in backends include `sdl`, `null` and `qt`.

## Renderers

Renderers subclass `Renderer`:

```python
from engine.renderers import Renderer, register_renderer

class FancyRenderer(Renderer):
    def clear(self, color=(0, 0, 0)):
        pass
    def draw_scene(self, scene, camera=None):
        pass
    def present(self):
        pass
    def close(self):
        pass

register_renderer('fancy', FancyRenderer)
```
The engine provides `opengl`, `null` and `sdl` renderers out of the box. Additional renderers can be installed via plugins.

## Loading Plugins

Plugin modules placed in `~/.sage_plugins` are discovered automatically.
Additional search paths can be specified in `sage.toml`:
```toml
[plugins]
extra = ["~/editor_plugins"]
```
Environment variables such as `SAGE_PLUGIN_DIR`, `SAGE_PLUGINS`,
`SAGE_ENGINE_PLUGIN_DIR` and `SAGE_EDITOR_PLUGIN_DIR` are still
supported but no longer required.
A module
can export an object named `plugin` deriving from `PluginBase` or define
`init_engine(engine)` and `init_editor(editor)` functions. These hooks may be
coroutines and are awaited when loaded.
Missing dependencies raise a warning so the plugin is skipped instead of
crashing the engine.

Plugins can also be installed via Python entry points. Call
`engine.plugins.load_plugins('engine', engine_instance)` to initialise them
manually if needed.
