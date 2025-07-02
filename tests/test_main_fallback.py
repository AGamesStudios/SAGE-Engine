import importlib
import types
import sys


def test_main_falls_back_to_runtime(monkeypatch):
    calls = []
    runtime_mod = types.ModuleType('engine.runtime')
    runtime_mod.main = lambda argv=None: calls.append(argv)
    monkeypatch.setitem(sys.modules, 'engine.runtime', runtime_mod)
    sys.modules.pop('sage_editor', None)
    import main
    importlib.reload(main)
    main.main(['data'])
    assert calls == [['data']]
