from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import math
import operator

@register_action('ModifyVariable', [('name', 'variable'), ('op', 'value'), ('value', 'value')])
class ModifyVariable(Action):
    """Modify a numeric variable using ``op`` and ``value``."""

    def __init__(self, name, op='+', value=0):
        self.name = name
        self.op = op
        self.value = value

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        current = engine.events.variables.get(self.name, 0)
        ops = {
            '+': operator.add,
            '-': operator.sub,
            '*': operator.mul,
            '/': operator.truediv,
        }
        func = ops.get(self.op, operator.add)
        try:
            engine.events.variables[self.name] = func(current, val)
        except Exception:
            logger.exception('ModifyVariable error')


