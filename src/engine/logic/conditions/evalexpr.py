import ast

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
            tree = ast.parse(self.expr, mode='eval')
            for node in ast.walk(tree):
                if isinstance(node, (ast.Call, ast.Import, ast.ImportFrom, ast.Lambda, ast.FunctionDef, ast.AsyncFunctionDef)):
                    raise ValueError('unsafe expression')
                if isinstance(node, ast.Attribute) and node.attr.startswith('_'):
                    raise ValueError('unsafe attribute')
            code = compile(tree, '<EvalExpr>', 'eval')
            return bool(eval(code, {'__builtins__': {}}, env))
        except Exception:
            logger.exception('EvalExpr error')
            return False
