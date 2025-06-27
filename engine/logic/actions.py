# Built-in actions for SAGE Logic
from .base import Action, register_action, resolve_value
from ..log import logger

@register_action('Move', [('obj', 'object', 'target'), ('dx', 'value'), ('dy', 'value')])
class Move(Action):
    """Move an object by ``dx`` and ``dy`` each frame."""
    def __init__(self, obj, dx=0, dy=0):
        self.obj = obj
        self.dx = dx
        self.dy = dy
    def execute(self, engine, scene, dt):
        self.obj.x += resolve_value(self.dx, engine)
        self.obj.y += resolve_value(self.dy, engine)

@register_action('SetPosition', [('obj', 'object', 'target'), ('x', 'value'), ('y', 'value')])
class SetPosition(Action):
    """Directly set the object's position."""
    def __init__(self, obj, x=0, y=0):
        self.obj = obj
        self.x = x
        self.y = y
    def execute(self, engine, scene, dt):
        self.obj.x = resolve_value(self.x, engine)
        self.obj.y = resolve_value(self.y, engine)

@register_action('Destroy', [('obj', 'object', 'target')])
class Destroy(Action):
    """Remove the object from the scene."""
    def __init__(self, obj):
        self.obj = obj
    def execute(self, engine, scene, dt):
        if hasattr(scene, 'remove_object'):
            scene.remove_object(self.obj)

@register_action('Print', [('text', 'value')])
class Print(Action):
    """Log a formatted message."""
    def __init__(self, text):
        self.text = text
    def execute(self, engine, scene, dt):
        if self.text is None:
            logger.warning('Print action missing text')
            return
        value = resolve_value(self.text, engine)
        if not isinstance(value, str):
            value = str(value)
        try:
            msg = value.format(**engine.events.variables)
        except Exception:
            msg = value
        logger.info(msg)

_SOUND_CACHE = {}

@register_action('PlaySound', [('path', 'value')])
class PlaySound(Action):
    """Play a sound file using simpleaudio if available."""
    def __init__(self, path):
        self.path = path
    def execute(self, engine, scene, dt):
        sound = _SOUND_CACHE.get(self.path)
        if sound is None:
            try:
                import simpleaudio
                sound = simpleaudio.WaveObject.from_wave_file(self.path)
                _SOUND_CACHE[self.path] = sound
            except Exception as exc:
                logger.warning('Failed to load sound %s: %s', self.path, exc)
                return
        try:
            sound.play()
        except Exception as exc:
            logger.warning('Failed to play sound %s: %s', self.path, exc)

@register_action('Spawn', [('image', 'value'), ('x', 'value'), ('y', 'value')])
class Spawn(Action):
    """Spawn a new sprite into the scene."""
    def __init__(self, image, x=0, y=0):
        self.image = image
        self.x = x
        self.y = y
    def execute(self, engine, scene, dt):
        try:
            from engine import GameObject
        except Exception:
            return
        obj = GameObject(self.image, resolve_value(self.x, engine), resolve_value(self.y, engine))
        obj.settings = {}
        if hasattr(scene, 'add_object'):
            scene.add_object(obj)

@register_action('SetVariable', [('name', 'value'), ('value', 'value')])
class SetVariable(Action):
    """Set an event variable."""
    def __init__(self, name, value):
        self.name = name
        self.value = value
    def execute(self, engine, scene, dt):
        engine.events.variables[self.name] = resolve_value(self.value, engine)

@register_action('ModifyVariable', [('name', 'value'), ('op', 'value'), ('value', 'value')])
class ModifyVariable(Action):
    """Modify a numeric variable with an operation."""
    OPS = {
        '+': lambda a, b: a + b,
        '-': lambda a, b: a - b,
        '*': lambda a, b: a * b,
        '/': lambda a, b: a / b if b != 0 else a,
    }
    def __init__(self, name, op, value):
        self.name = name
        self.op = op
        self.value = value
    def execute(self, engine, scene, dt):
        cur = engine.events.variables.get(self.name)
        if not isinstance(cur, (int, float)):
            logger.warning('Variable %s is not numeric; ModifyVariable skipped', self.name)
            return
        try:
            cur = float(cur)
            val = float(resolve_value(self.value, engine))
            func = self.OPS.get(self.op, lambda a, b: a)
            engine.events.variables[self.name] = func(cur, val)
        except Exception:
            logger.warning('Failed to modify variable %s', self.name)

@register_action('SetZoom', [('camera', 'object', 'target', ['camera']), ('zoom', 'value')])
class SetZoom(Action):
    """Set the zoom level of a camera."""
    def __init__(self, camera, zoom):
        self.camera = camera
        self.zoom = zoom
    def execute(self, engine, scene, dt):
        self.camera.zoom = resolve_value(self.zoom, engine)

@register_action('PanCamera', [('camera', 'object', 'target', ['camera']), ('dx', 'value'), ('dy', 'value')])
class PanCamera(Action):
    """Offset the camera position."""
    def __init__(self, camera, dx=0, dy=0):
        self.camera = camera
        self.dx = dx
        self.dy = dy
    def execute(self, engine, scene, dt):
        self.camera.x += resolve_value(self.dx, engine)
        self.camera.y += resolve_value(self.dy, engine)

@register_action('CenterCamera', [('camera', 'object', 'target', ['camera']), ('obj', 'object', 'target')])
class CenterCamera(Action):
    """Center the camera on the target object."""
    def __init__(self, camera, obj):
        self.camera = camera
        self.obj = obj
    def execute(self, engine, scene, dt):
        self.camera.x = self.obj.x
        self.camera.y = self.obj.y

@register_action('EnableEvent', [('name', 'value')])
class EnableEvent(Action):
    """Enable an event by name."""
    def __init__(self, name):
        self.name = name
    def execute(self, engine, scene, dt):
        engine.events.enable_event(resolve_value(self.name, engine))

@register_action('DisableEvent', [('name', 'value')])
class DisableEvent(Action):
    """Disable an event by name."""
    def __init__(self, name):
        self.name = name
    def execute(self, engine, scene, dt):
        engine.events.disable_event(resolve_value(self.name, engine))

@register_action('ResetEvent', [('name', 'value')])
class ResetEvent(Action):
    """Reset an event so it can trigger again."""
    def __init__(self, name):
        self.name = name
    def execute(self, engine, scene, dt):
        engine.events.reset_event(resolve_value(self.name, engine))

__all__ = [
    'Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn',
    'SetVariable', 'ModifyVariable', 'SetZoom', 'PanCamera', 'CenterCamera',
    'EnableEvent', 'DisableEvent', 'ResetEvent'
]
