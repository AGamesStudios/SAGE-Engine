from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('SetVariable', [('name', 'variable'), ('value', 'value')])
class SetVariable(Action):
    """Assign ``value`` to a variable."""

    def __init__(self, name, value):
        self.name = name
        self.value = value

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        engine.events.variables[self.name] = val


