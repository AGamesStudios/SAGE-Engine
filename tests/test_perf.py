import importlib


def test_detect_low_perf_env(monkeypatch):
    monkeypatch.setenv("SAGE_LOW_PERF", "1")
    perf = importlib.import_module("sage_engine.perf")
    importlib.reload(perf)
    assert perf.detect_low_perf() is True
    assert perf.is_low_perf() is True
