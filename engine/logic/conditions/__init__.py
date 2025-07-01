"""Placeholder module for SAGE Logic conditions.

No built-in conditions are provided. Games and plugins should define their own
:class:`Condition` subclasses and register them with :func:`register_condition`.
"""

from ..base import Condition, register_condition, resolve_value  # re-export for convenience


@register_condition('OnStart', [])
class OnStart(Condition):
    """True on the first update after the event system is created or reset."""

    def __init__(self):
        self.triggered = False

    def check(self, engine, scene, dt):
        if not self.triggered:
            self.triggered = True
            return True
        return False

    def reset(self):
        self.triggered = False


__all__ = [
    'OnStart',
    'KeyPressed',
    'KeyReleased',
    'InputState',
    'ObjectVisible',
    'VariableCompare',
    'ObjectVariableCompare',
]


@register_condition('KeyPressed', [('key', 'value'), ('device', 'value')])
class KeyPressed(Condition):
    """Triggered when a key or mouse button is pressed."""

    def __init__(self, key, device='keyboard'):
        self.key = key
        self.device = device
        self.prev = False

    def check(self, engine, scene, dt):
        if self.device == 'mouse':
            down = engine.input.is_button_down(self.key)
        else:
            down = engine.input.is_key_down(self.key)
        triggered = down and not self.prev
        self.prev = down
        return triggered

    def reset(self):
        self.prev = False


@register_condition('KeyReleased', [('key', 'value'), ('device', 'value')])
class KeyReleased(Condition):
    """Triggered when a key or mouse button is released."""

    def __init__(self, key, device='keyboard'):
        self.key = key
        self.device = device
        self.prev = False

    def check(self, engine, scene, dt):
        if self.device == 'mouse':
            down = engine.input.is_button_down(self.key)
        else:
            down = engine.input.is_key_down(self.key)
        triggered = not down and self.prev
        self.prev = down
        return triggered

    def reset(self):
        self.prev = False


@register_condition('InputState', [('device', 'value'), ('code', 'value'), ('state', 'value')])
class InputState(Condition):
    """Check if a key or button is currently pressed or released."""

    def __init__(self, device='keyboard', code=0, state='down'):
        self.device = device
        self.code = code
        self.state = state

    def check(self, engine, scene, dt):
        if self.device == 'mouse':
            down = engine.input.is_button_down(self.code)
        else:
            down = engine.input.is_key_down(self.code)
        if self.state in ('down', 'pressed'):
            return down
        return not down


@register_condition('ObjectVisible', [('target', 'object'), ('visible', 'value')])
class ObjectVisible(Condition):
    """Check if an object's alpha visibility matches ``visible``."""

    def __init__(self, target, visible=True):
        self.target = target
        self.visible = bool(visible)

    def check(self, engine, scene, dt):
        alpha = getattr(self.target, 'alpha', 1.0)
        vis = getattr(self.target, 'visible', alpha > 0)
        return (vis and alpha > 0) == self.visible


@register_condition('VariableCompare', [('name', 'variable'), ('op', 'value'), ('value', 'value')])
class VariableCompare(Condition):
    """Compare a variable using ``op`` against ``value``."""

    def __init__(self, name, op='==', value=0):
        self.name = name
        self.op = op
        self.value = value

    def check(self, engine, scene, dt):
        import operator
        val = engine.events.variables.get(self.name)
        target_val = resolve_value(self.value, engine)
        ops = {
            '==': operator.eq,
            '!=': operator.ne,
            '<': operator.lt,
            '<=': operator.le,
            '>': operator.gt,
            '>=': operator.ge,
        }
        func = ops.get(self.op, operator.eq)
        try:
            return func(val, target_val)
        except Exception:
            return False


@register_condition('ObjectVariableCompare', [('target', 'object'), ('name', 'value'), ('op', 'value'), ('value', 'value')])
class ObjectVariableCompare(Condition):
    """Compare an object's variable using ``op`` against ``value``."""

    def __init__(self, target, name, op='==', value=0):
        self.target = target
        self.name = name
        self.op = op
        self.value = value

    def check(self, engine, scene, dt):
        import operator
        val = self.target.get_variable(self.name)
        target_val = resolve_value(self.value, engine)
        ops = {
            '==': operator.eq,
            '!=': operator.ne,
            '<': operator.lt,
            '<=': operator.le,
            '>': operator.gt,
            '>=': operator.ge,
        }
        func = ops.get(self.op, operator.eq)
        try:
            return func(val, target_val)
        except Exception:
            return False

