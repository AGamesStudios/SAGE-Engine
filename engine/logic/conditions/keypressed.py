from ..base import Condition, register_condition

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


