from ..base import Action, register_action, resolve_value


@register_action('SetZOrder', [('target', 'object'), ('z', 'value')])
class SetZOrder(Action):
    """Update the object's rendering order."""

    def __init__(self, target, z=0):
        self.target = target
        self.z = z

    def execute(self, engine, scene, dt):
        self.target.z = float(resolve_value(self.z, engine))
        if scene:
            scene.sort_objects()


