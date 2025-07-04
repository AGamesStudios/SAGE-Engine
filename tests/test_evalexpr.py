import types
from engine.logic.conditions.evalexpr import EvalExpr


def test_evalexpr_disallows_builtins():
    engine = types.SimpleNamespace(events=types.SimpleNamespace(variables={}))
    cond = EvalExpr('__import__("os")')
    assert cond.check(engine, None, 0) is False
