import types
from engine.logic.conditions.evalexpr import EvalExpr


def test_evalexpr_disallows_builtins():
    engine = types.SimpleNamespace(events=types.SimpleNamespace(variables={}))
    cond = EvalExpr('__import__("os")')
    assert cond.check(engine, None, 0) is False


def test_evalexpr_variable_injection_blocked():
    vars = {'__builtins__': {'__import__': __import__}}
    engine = types.SimpleNamespace(events=types.SimpleNamespace(variables=vars))
    cond = EvalExpr('__import__("os")')
    assert cond.check(engine, None, 0) is False
