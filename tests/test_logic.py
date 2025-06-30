import types
import importlib.util
import pathlib
import sys

# Create minimal engine package so logic modules with relative imports load.
engine_pkg = types.ModuleType('engine')
sys.modules.setdefault('engine', engine_pkg)
logic_pkg = types.ModuleType('engine.logic')
logic_pkg.__path__ = []
sys.modules.setdefault('engine.logic', logic_pkg)
log_mod = types.ModuleType('engine.log')
log_mod.logger = types.SimpleNamespace(
    info=lambda *a, **k: None,
    warning=lambda *a, **k: None,
    exception=lambda *a, **k: None,
    debug=lambda *a, **k: None,
)
sys.modules['engine.log'] = log_mod
lang_mod = types.ModuleType('engine.lang')
lang_mod.LANGUAGES = {}
sys.modules['engine.lang'] = lang_mod

base_path = pathlib.Path('engine/logic')

spec = importlib.util.spec_from_file_location('engine.logic.base', base_path / 'base.py')
logic_base = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = logic_base
spec.loader.exec_module(logic_base)

spec = importlib.util.spec_from_file_location('engine.logic.conditions', base_path / 'conditions.py')
logic_conditions = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = logic_conditions
spec.loader.exec_module(logic_conditions)

spec = importlib.util.spec_from_file_location('engine.logic.actions', base_path / 'actions.py')
logic_actions = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = logic_actions
spec.loader.exec_module(logic_actions)

VariableCompare = logic_conditions.VariableCompare
ObjectVisible = logic_conditions.ObjectVisible
SetVariable = logic_actions.SetVariable
ModifyVariable = logic_actions.ModifyVariable
ShowObject = logic_actions.ShowObject
HideObject = logic_actions.HideObject

class DummyObj:
    def __init__(self):
        self.x = 0.0
        self.y = 0.0
        self.alpha = 1.0


def test_variable_actions_and_condition():
    es = types.SimpleNamespace(variables={'score': 0})
    engine = types.SimpleNamespace(events=es)
    SetVariable('score', 5).execute(engine, None, 0)
    assert es.variables['score'] == 5
    ModifyVariable('score', '+', 2).execute(engine, None, 0)
    assert es.variables['score'] == 7
    cond = VariableCompare('score', '>=', 7)
    assert cond.check(engine, None, 0)


def test_visibility_actions_and_condition():
    obj = DummyObj()
    es = types.SimpleNamespace(variables={})
    engine = types.SimpleNamespace(events=es)
    visible_cond = ObjectVisible(obj, True)
    assert visible_cond.check(engine, None, 0)
    HideObject(obj).execute(engine, None, 0)
    assert obj.alpha == 0.0
    assert not visible_cond.check(engine, None, 0)
    ShowObject(obj).execute(engine, None, 0)
    assert obj.alpha == 1.0
    assert visible_cond.check(engine, None, 0)
