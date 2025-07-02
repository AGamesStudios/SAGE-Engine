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
```

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
