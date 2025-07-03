from ..base import Condition, register_condition

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


