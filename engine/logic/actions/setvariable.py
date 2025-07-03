from ..base import Action, register_action, resolve_value


@register_action('SetVariable', [('name', 'variable'), ('value', 'value'), ('private', 'value')])
class SetVariable(Action):
    """Assign ``value`` to a variable."""

    def __init__(self, name, value, private=False):
        self.name = name
        self.value = value
        self.private = bool(private)

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        vars_dict = engine.events.variables
        if hasattr(vars_dict, "set"):
            vars_dict.set(self.name, val, self.private)
        elif hasattr(vars_dict, "lock"):
            with vars_dict.lock:
                vars_dict[self.name] = val
        else:
            vars_dict[self.name] = val


