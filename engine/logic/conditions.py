"""Placeholder module for SAGE Logic conditions.

No built-in conditions are provided. Games and plugins should define their own
:class:`Condition` subclasses and register them with :func:`register_condition`.
"""

from .base import Condition, register_condition  # re-export for convenience


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

