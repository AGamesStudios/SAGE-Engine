from ..base import Condition, register_condition
from ...utils.log import logger

@register_condition('EvalExpr', [('expr', 'value')])
class EvalExpr(Condition):
    """Evaluate a Python expression as a condition."""

    def __init__(self, expr: str):
        self.expr = expr

    def check(self, engine, scene, dt):
        env = {'engine': engine, 'scene': scene, 'dt': dt}
        variables = getattr(engine, 'events', {}).variables
        if isinstance(variables, dict):
            env.update({k: v for k, v in variables.items() if k != '__builtins__'})
        else:
            env.update(variables)
        try:
            return bool(eval(self.expr, {'__builtins__': {}}, env))
        except Exception:
            logger.exception('EvalExpr error')
            return False
