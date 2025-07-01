from ..base import Action, register_action, resolve_value


@register_action('Flip', [('target', 'object'), ('flip_x', 'value'), ('flip_y', 'value')])
class Flip(Action):
    """Flip object horizontally or vertically."""

    def __init__(self, target, flip_x=False, flip_y=False):
        self.target = target
        self.flip_x = flip_x
        self.flip_y = flip_y

    def execute(self, engine, scene, dt):
        self.target.flip_x = bool(resolve_value(self.flip_x, engine))
        self.target.flip_y = bool(resolve_value(self.flip_y, engine))


