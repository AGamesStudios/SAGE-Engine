from ..base import Condition, register_condition

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


