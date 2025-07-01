from ..base import Action, register_action, resolve_value


@register_action('SetObjectVariable', [('target', 'object'), ('name', 'value'), ('value', 'value'), ('type', 'value')])
class SetObjectVariable(Action):
    """Assign ``value`` to an object's variable."""

    def __init__(self, target, name, value=None, type='any'):
        self.target = target
        self.name = name
        self.value = value
        self.type = type

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        if hasattr(self.target, 'add_variable'):
            self.target.add_variable(str(self.name), val, str(self.type))


