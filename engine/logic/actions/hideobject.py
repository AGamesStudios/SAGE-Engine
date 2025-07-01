from ..base import Action, register_action


@register_action('HideObject', [('target', 'object')])
class HideObject(Action):
    """Hide an object by setting ``alpha`` to 0."""

    def __init__(self, target):
        self.target = target

    def execute(self, engine, scene, dt):
        if hasattr(self.target, 'visible'):
            self.target.visible = False
        if hasattr(self.target, 'alpha'):
            self.target.alpha = 0.0


