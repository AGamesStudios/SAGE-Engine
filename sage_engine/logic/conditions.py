# Built-in conditions
from .base import Condition, register_condition

@register_condition('InputState')
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


@register_condition('KeyPressed')
class KeyPressed(InputState):
    def __init__(self, key):
        super().__init__('keyboard', key, 'down')

@register_condition('Collision')
class Collision(Condition):
    def __init__(self, obj_a, obj_b):
        self.obj_a = obj_a
        self.obj_b = obj_b

    def check(self, engine, scene, dt):
        return self.obj_a.rect().colliderect(self.obj_b.rect())

@register_condition('Timer')
class Timer(Condition):
    """True every `duration` seconds."""
    def __init__(self, duration):
        self.duration = duration
        self.elapsed = 0.0

    def check(self, engine, scene, dt):
        self.elapsed += dt
        if self.elapsed >= self.duration:
            self.elapsed -= self.duration
            return True
        return False

@register_condition('KeyReleased')
class KeyReleased(InputState):
    """True once when the key transitions from pressed to released."""

    def __init__(self, key):
        super().__init__('keyboard', key, 'released')

@register_condition('MouseButton')
class MouseButton(InputState):
    """Check mouse button state ('down' or 'up')."""

    def __init__(self, button, state='down'):
        super().__init__('mouse', button, state)

@register_condition('Always')
class Always(Condition):
    """Condition that is always true."""

    def check(self, engine, scene, dt):
        return True

@register_condition('OnStart')
class OnStart(Condition):
    """True only on the first frame."""
    def __init__(self):
        self.triggered = False

    def check(self, engine, scene, dt):
        if not self.triggered:
            self.triggered = True
            return True
        return False

@register_condition('EveryFrame')
class EveryFrame(Condition):
    """Alias for Always to clarify intent."""
    def check(self, engine, scene, dt):
        return True

@register_condition('VariableCompare')
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
                return cmp(float(val), float(self.value))
            if isinstance(val, bool):
                if self.op not in ('==', '!='):
                    return False
                ref = self.value
                if isinstance(ref, str):
                    ref = ref.lower() in ('true', '1', 'yes')
                return cmp(val, bool(ref))
            # strings or other types use string comparison for == and !=
            if self.op in ('==', '!='):
                return cmp(str(val), str(self.value))
        except Exception:
            pass
        return False

__all__ = ['KeyPressed','KeyReleased','MouseButton','InputState','Collision','Timer','Always','OnStart','EveryFrame','VariableCompare']
