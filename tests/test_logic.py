import types
import importlib.util
import importlib
import pathlib
import sys
import pytest

# Create minimal engine package so logic modules with relative imports load.
_orig_engine = sys.modules.get('engine')
_orig_logic = sys.modules.get('engine.logic')
engine_pkg = types.ModuleType('engine')
sys.modules['engine'] = engine_pkg
logic_pkg = types.ModuleType('engine.logic')
logic_pkg.__path__ = []
sys.modules['engine.logic'] = logic_pkg
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
utils_mod = types.ModuleType('engine.utils')
sys.modules['engine.utils'] = utils_mod
sys.modules['engine.utils.log'] = log_mod

ROOT = pathlib.Path(__file__).resolve().parent.parent
base_path = ROOT / 'src' / 'sage_engine' / 'logic'

spec = importlib.util.spec_from_file_location('engine.logic.base', base_path / 'base.py')
logic_base = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = logic_base
spec.loader.exec_module(logic_base)

spec = importlib.util.spec_from_file_location('engine.logic.conditions', base_path / 'conditions' / '__init__.py')
logic_conditions = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = logic_conditions
spec.loader.exec_module(logic_conditions)

spec = importlib.util.spec_from_file_location('engine.logic.actions', base_path / 'actions' / '__init__.py')
logic_actions = importlib.util.module_from_spec(spec)
sys.modules[spec.name] = logic_actions
spec.loader.exec_module(logic_actions)

VariableCompare = logic_conditions.VariableCompare
ObjectVisible = logic_conditions.ObjectVisible
ObjectVariableCompare = logic_conditions.ObjectVariableCompare
EvalExpr = logic_conditions.EvalExpr
SetVariable = logic_actions.SetVariable
ModifyVariable = logic_actions.ModifyVariable
SetObjectVariable = logic_actions.SetObjectVariable
ModifyObjectVariable = logic_actions.ModifyObjectVariable
CallFunction = logic_actions.CallFunction
ShowObject = logic_actions.ShowObject
HideObject = logic_actions.HideObject

# restore original engine modules so other tests can import the real package
if _orig_engine is not None:
    sys.modules['engine'] = _orig_engine
else:
    sys.modules.pop('engine', None)
if _orig_logic is not None:
    sys.modules['engine.logic'] = _orig_logic
else:
    sys.modules.pop('engine.logic', None)
importlib.invalidate_caches()

class DummyObj:
    def __init__(self):
        self.x = 0.0
        self.y = 0.0
        self.alpha = 1.0
        self.visible = True
        self.variables = {}

    def add_variable(self, name, value=None, typ='any', public=True):
        self.variables[name] = value

    def get_variable(self, name, default=None):
        return self.variables.get(name, default)

    def set_variable(self, name, value):
        self.variables[name] = value


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
    assert obj.visible is False
    assert not visible_cond.check(engine, None, 0)
    ShowObject(obj).execute(engine, None, 0)
    assert obj.alpha == 1.0
    assert obj.visible is True
    assert visible_cond.check(engine, None, 0)


def test_object_variable_actions_and_condition():
    obj = DummyObj()
    es = types.SimpleNamespace(variables={})
    engine = types.SimpleNamespace(events=es)
    SetObjectVariable(obj, "hp", 10, "int").execute(engine, None, 0)
    assert obj.get_variable("hp") == 10
    ModifyObjectVariable(obj, "hp", '-', 3).execute(engine, None, 0)
    assert obj.get_variable("hp") == 7
    cond = ObjectVariableCompare(obj, "hp", '>=', 5)
    assert cond.check(engine, None, 0)


def test_callfunction_and_evalexpr():
    es = types.SimpleNamespace(variables={"v": 0})
    engine = types.SimpleNamespace(events=es)

    called = {}

    def foo(x):
        called["x"] = x

    CallFunction(foo, [5]).execute(engine, None, 0)
    assert called["x"] == 5

    SetVariable("v", 5).execute(engine, None, 0)
    cond = EvalExpr('engine.events.variables["v"] == 5')
    assert cond.check(engine, None, 0)


def test_private_variable_support():
    es = types.SimpleNamespace(variables=logic_base.VariableStore())
    engine = types.SimpleNamespace(events=es)

    SetVariable("secret", 42, private=True).execute(engine, None, 0)
    assert es.variables["secret"] == 42
    assert es.variables.is_private("secret")


def test_input_axis_condition():
    class DummyBackend:
        def get_axis_value(self, aid):
            return 0.7

    engine = types.SimpleNamespace(input=DummyBackend())
    cond = logic_conditions.InputAxis(axis_id=0, threshold=0.5, comparison='>=')
    assert cond.check(engine, None, 0)

    class BadBackend:
        def get_axis_value(self, aid):
            return None

    engine_bad = types.SimpleNamespace(input=BadBackend())
    cond_bad = logic_conditions.InputAxis(axis_id=1, threshold=0.5)
    with pytest.raises(ValueError):
        cond_bad.check(engine_bad, None, 0)


def teardown_module(module):
    import importlib
    for name in [
        'engine.logic',
        'engine.logic.base',
        'engine.logic.actions',
        'engine.logic.conditions',
    ]:
        if name in sys.modules:
            del sys.modules[name]
    importlib.invalidate_caches()
