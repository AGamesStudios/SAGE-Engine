from ..base import Condition, register_condition, resolve_value

@register_condition('ObjectVariableCompare', [('target', 'object'), ('name', 'value'), ('op', 'value'), ('value', 'value')])
class ObjectVariableCompare(Condition):
    """Compare an object's variable using ``op`` against ``value``."""

    def __init__(self, target, name, op='==', value=0):
        self.target = target
        self.name = name
        self.op = op
        self.value = value

    def check(self, engine, scene, dt):
        import operator
        val = self.target.get_variable(self.name)
        target_val = resolve_value(self.value, engine)
        ops = {
            '==': operator.eq,
            '!=': operator.ne,
            '<': operator.lt,
            '<=': operator.le,
            '>': operator.gt,
            '>=': operator.ge,
        }
        func = ops.get(self.op, operator.eq)
        try:
            return func(val, target_val)
        except Exception:
            return False

