from ..base import Action, register_action, resolve_value
from ...utils.log import logger
import operator

@register_action('ModifyObjectVariable', [('target', 'object'), ('name', 'value'), ('op', 'value'), ('value', 'value')])
class ModifyObjectVariable(Action):
    """Modify a numeric object variable using ``op`` and ``value``."""

    def __init__(self, target, name, op='+', value=0):
        self.target = target
        self.name = name
        self.op = op
        self.value = value

    def execute(self, engine, scene, dt):
        val = resolve_value(self.value, engine)
        current = self.target.get_variable(self.name, 0)
        ops = {
            '+': operator.add,
            '-': operator.sub,
            '*': operator.mul,
            '/': operator.truediv,
        }
        func = ops.get(self.op, operator.add)
        try:
            self.target.set_variable(self.name, func(current, val))
        except Exception:
            logger.exception('ModifyObjectVariable error')


