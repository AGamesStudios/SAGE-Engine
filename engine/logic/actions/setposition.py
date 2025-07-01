from ..base import Action, register_action, resolve_value


@register_action('SetPosition', [('target', 'object'), ('x', 'value'), ('y', 'value')])
class SetPosition(Action):
    """Set object position."""

    def __init__(self, target, x=0.0, y=0.0):
        self.target = target
        self.x = x
        self.y = y

    def execute(self, engine, scene, dt):
        self.target.x = float(resolve_value(self.x, engine))
        self.target.y = float(resolve_value(self.y, engine))


