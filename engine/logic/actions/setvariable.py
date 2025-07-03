from ..base import Action, register_action, resolve_value


@register_action('SetVariable', [('name', 'variable'), ('value', 'value')])
class SetVariable(Action):
    """Assign ``value`` to a variable."""

    def __init__(self, name, value):
        self.name = name
        self.value = value

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        vars_dict = engine.events.variables
        if hasattr(vars_dict, "lock"):
            with vars_dict.lock:
                vars_dict[self.name] = val
        else:
            vars_dict[self.name] = val


