# Built-in conditions
from .base import Condition, register_condition, resolve_value

@register_condition('InputState', [
    ('device', 'value'),
    ('code', 'value'),
    ('state', 'value'),
])
class InputState(Condition):
    """Check a key or mouse button with optional device selection."""

    def __init__(self, device='keyboard', code=None, state='down'):
        self.device = device
        self.code = code
        self.state = state
        self.prev = False

    def check(self, engine, scene, dt):
        inp = engine.input
        if self.device == 'mouse':
            pressed = inp.is_button_down(self.code) if self.code is not None else bool(inp._buttons)
        else:
            pressed = inp.is_key_down(self.code) if self.code is not None else bool(inp._keys)
        if self.state in ('pressed', 'released'):
            result = (not self.prev and pressed) if self.state == 'pressed' else (self.prev and not pressed)
            self.prev = pressed
            return result
        return pressed if self.state == 'down' else not pressed


@register_condition('KeyPressed', [
    ('key', 'value'),
    ('device', 'value'),
])
class KeyPressed(InputState):
    """True while the selected key or button is held down."""

    def __init__(self, key, device='keyboard'):
        super().__init__(device, key, 'down')

@register_condition('Collision', [
    ('obj_a', 'object', 'a'),
    ('obj_b', 'object', 'b'),
])
class Collision(Condition):
    def __init__(self, obj_a, obj_b):
        self.obj_a = obj_a
        self.obj_b = obj_b

    def check(self, engine, scene, dt):
        ax, ay, aw, ah = self.obj_a.rect()
        bx, by, bw, bh = self.obj_b.rect()
        return not (
            ax + aw <= bx or ax >= bx + bw or
            ay + ah <= by or ay >= by + bh
        )

@register_condition('AfterTime', [
    ('seconds', 'value'),
    ('minutes', 'value'),
    ('hours', 'value'),
])
class AfterTime(Condition):
    """True once after the specified time has elapsed."""

    def __init__(self, seconds=0.0, minutes=0.0, hours=0.0):
        seconds = 0.0 if seconds is None else float(seconds)
        minutes = 0.0 if minutes is None else float(minutes)
        hours = 0.0 if hours is None else float(hours)
        self.target = seconds + minutes * 60 + hours * 3600
        self.elapsed = 0.0
        self.triggered = False

    def check(self, engine, scene, dt):
        if self.triggered:
            return False
        self.elapsed += dt
        if self.elapsed >= self.target:
            self.triggered = True
            return True
        return False

@register_condition('KeyReleased', [
    ('key', 'value'),
    ('device', 'value'),
])
class KeyReleased(InputState):
    """True once when the key transitions from pressed to released."""

    def __init__(self, key, device='keyboard'):
        super().__init__(device, key, 'released')

@register_condition('MouseButton', [
    ('button', 'value'),
    ('state', 'value'),
])
class MouseButton(InputState):
    """Check mouse button state ('down' or 'up')."""

    def __init__(self, button, state='down'):
        super().__init__('mouse', button, state)


@register_condition('OnStart', [])
class OnStart(Condition):
    """True only on the first frame."""
    def __init__(self):
        self.triggered = False

    def check(self, engine, scene, dt):
        if not self.triggered:
            self.triggered = True
            return True
        return False

@register_condition('EveryFrame', [])
class EveryFrame(Condition):
    """True on every engine tick."""
    def check(self, engine, scene, dt):
        return True

@register_condition('VariableCompare', [
    ('name', 'value'),
    ('op', 'value'),
    ('value', 'value'),
])
class VariableCompare(Condition):
    """Compare a variable to a value using an operator."""
    OPS = {
        '==': lambda a, b: a == b,
        '!=': lambda a, b: a != b,
        '<': lambda a, b: a < b,
        '<=': lambda a, b: a <= b,
        '>': lambda a, b: a > b,
        '>=': lambda a, b: a >= b,
    }

    def __init__(self, name, op, value):
        self.name = name
        self.op = op
        self.value = value

    def check(self, engine, scene, dt):
        val = engine.events.variables.get(self.name)
        cmp = self.OPS.get(self.op, lambda a, b: False)
        try:
            if isinstance(val, (int, float)):
                return cmp(float(val), float(resolve_value(self.value, engine)))
            if isinstance(val, bool):
                if self.op not in ('==', '!='):
                    return False
                ref = resolve_value(self.value, engine)
                if isinstance(ref, str):
                    ref = ref.lower() in ('true', '1', 'yes')
                return cmp(val, bool(ref))
            # text and other types are not supported
        except Exception:
            pass
        return False


@register_condition('ZoomAbove', [
    ('camera', 'object', 'target', ['camera']),
    ('value', 'value'),
])
class ZoomAbove(Condition):
    """True when the camera zoom is greater than ``value``."""

    def __init__(self, camera, value):
        self.camera = camera
        self.value = value

    def check(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        try:
            return self.camera.zoom > float(val)
        except Exception:
            return False


@register_condition('EventTriggered', [('name', 'value')])
class EventTriggered(Condition):
    """True when the named event has already executed."""

    def __init__(self, name):
        self.name = name

    def check(self, engine, scene, dt):
        target = resolve_value(self.name, engine)
        evt = None
        if engine.events:
            evt = engine.events.get_event(target)
        if evt is None:
            for obj in getattr(scene, 'objects', []):
                es = getattr(obj, 'event_system', None)
                if es:
                    evt = es.get_event(target)
                    if evt is not None:
                        break
        return evt.triggered if evt else False

__all__ = [
    'KeyPressed', 'KeyReleased', 'MouseButton', 'InputState',
    'Collision', 'AfterTime', 'OnStart', 'EveryFrame',
    'VariableCompare', 'ZoomAbove', 'EventTriggered'
]
