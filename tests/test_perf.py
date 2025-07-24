import importlib


def test_detect_low_perf_env(monkeypatch):
    monkeypatch.setenv("SAGE_LOW_PERF", "1")
    perf = importlib.import_module("sage_engine.perf")
    importlib.reload(perf)
    assert perf.detect_low_perf() is True
    assert perf.is_low_perf() is True


def test_check_memory_without_modules(monkeypatch):
    import builtins
    import sys

    orig_import = builtins.__import__

    def fake_import(name, *args, **kwargs):
        if name == "resource":
            raise ImportError
        return orig_import(name, *args, **kwargs)
    monkeypatch.setattr(builtins, "__import__", fake_import)
    sys.modules.pop("psutil", None)
    monkeypatch.setitem(sys.modules, "psutil", None)
    perf = importlib.import_module("sage_engine.perf")
    perf = importlib.reload(perf)
    assert perf.check_memory() is False
