import importlib
import types
import sys


def test_main_falls_back_to_runtime(monkeypatch):
    calls = []
    runtime_mod = types.ModuleType('engine.runtime')
    runtime_mod.main = lambda argv=None: calls.append(argv)
    monkeypatch.setitem(sys.modules, 'engine.runtime', runtime_mod)
    import main
    importlib.reload(main)
    monkeypatch.setattr(main, '_load_editor_main', lambda: None)
    main.main(['data'])
    assert calls == [['data']]
