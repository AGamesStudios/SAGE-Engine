# Built-in actions
from .base import Action, register_action
@register_action('Move', [
    ('obj', 'object', 'target'),
    ('dx', 'value', None),
    ('dy', 'value', None),
])
class Move(Action):
    def __init__(self, obj, dx, dy):
        self.obj = obj
        self.dx = dx
        self.dy = dy

    def execute(self, engine, scene, dt):
        self.obj.x += self.dx
        self.obj.y += self.dy

@register_action('SetPosition', [
    ('obj', 'object', 'target'),
    ('x', 'value', None),
    ('y', 'value', None),
])
class SetPosition(Action):
    def __init__(self, obj, x, y):
        self.obj = obj
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        self.obj.x = self.x
        self.obj.y = self.y

@register_action('Destroy', [('obj', 'object', 'target')])
class Destroy(Action):
    def __init__(self, obj):
        self.obj = obj

    def execute(self, engine, scene, dt):
        if hasattr(scene, 'remove_object'):
            scene.remove_object(self.obj)

@register_action('Print', [('text', 'value', None)])
class Print(Action):
    def __init__(self, text):
        self.text = text

    def execute(self, engine, scene, dt):
        try:
            msg = self.text.format(**engine.events.variables)
        except Exception:
            msg = self.text
        print(msg)

_SOUND_CACHE = {}

@register_action('PlaySound', [('path', 'value', None)])
class PlaySound(Action):
    """Play a sound file using simpleaudio."""

    def __init__(self, path):
        self.path = path
        self.wave = None

    def execute(self, engine, scene, dt):
        sound = _SOUND_CACHE.get(self.path)
        if sound is None:
            try:
                import simpleaudio
                sound = simpleaudio.WaveObject.from_wave_file(self.path)
                _SOUND_CACHE[self.path] = sound
            except Exception as exc:
                print(f'Failed to load sound {self.path}: {exc}')
                return
        try:
            sound.play()
        except Exception as exc:
            print(f'Failed to play sound {self.path}: {exc}')

@register_action('Spawn', [
    ('image', 'value', None),
    ('x', 'value', None),
    ('y', 'value', None),
])
class Spawn(Action):
    """Spawn a new GameObject into the scene."""

    def __init__(self, image, x=0, y=0):
        self.image = image
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        try:
            from engine import GameObject
        except Exception:
            return
        obj = GameObject(self.image, self.x, self.y)
        obj.settings = {}
        if hasattr(scene, 'add_object'):
            scene.add_object(obj)

@register_action('SetVariable', [
    ('name', 'value', None),
    ('value', 'value', None),
])
class SetVariable(Action):
    """Set a variable in the event system."""
    def __init__(self, name, value):
        self.name = name
        self.value = value

    def execute(self, engine, scene, dt):
        engine.events.variables[self.name] = self.value

@register_action('ModifyVariable', [
    ('name', 'value', None),
    ('op', 'value', None),
    ('value', 'value', None),
])
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
            print(f'Variable {self.name} is not numeric; ModifyVariable skipped')
            return
        try:
            cur = float(cur)
            val = float(self.value)
            func = self.OPS.get(self.op, lambda a, b: a)
            engine.events.variables[self.name] = func(cur, val)
        except Exception:
            print(f'Failed to modify variable {self.name}')

__all__ = ['Move','SetPosition','Destroy','Print','PlaySound','Spawn','SetVariable','ModifyVariable']
