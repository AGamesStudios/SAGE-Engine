from ..base import Action, register_action, resolve_value


@register_action('RenameObject', [('target', 'object'), ('name', 'value')])
class RenameObject(Action):
    """Rename the given object."""

    def __init__(self, target, name):
        self.target = target
        self.name = name

    def execute(self, engine, scene, dt):
        self.target.name = str(resolve_value(self.name, engine))


