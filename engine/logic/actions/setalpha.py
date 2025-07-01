from ..base import Action, register_action, resolve_value


@register_action('SetAlpha', [('target', 'object'), ('alpha', 'value')])
class SetAlpha(Action):
    """Set object alpha transparency (0..1)."""

    def __init__(self, target, alpha=1.0):
        self.target = target
        self.alpha = alpha

    def execute(self, engine, scene, dt):
        self.target.alpha = float(resolve_value(self.alpha, engine))


