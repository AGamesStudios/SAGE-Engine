from ..base import Condition, register_condition
from ...utils.log import logger

@register_condition('EvalExpr', [('expr', 'value')])
class EvalExpr(Condition):
    """Evaluate a Python expression as a condition."""

    def __init__(self, expr: str):
        self.expr = expr

    def check(self, engine, scene, dt):
        env = {'engine': engine, 'scene': scene, 'dt': dt}
        env.update(getattr(engine, 'events', {}).variables)
        try:
            return bool(eval(self.expr, {'__builtins__': {}}, env))
        except Exception:
            logger.exception('EvalExpr error')
            return False
