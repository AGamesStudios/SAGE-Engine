from ..base import Action, register_action


@register_action('ShowObject', [('target', 'object')])
class ShowObject(Action):
    """Make an object visible by setting ``alpha`` to 1.0."""

    def __init__(self, target):
        self.target = target

    def execute(self, engine, scene, dt):
        if hasattr(self.target, 'visible'):
            self.target.visible = True
        if hasattr(self.target, 'alpha'):
            self.target.alpha = 1.0


