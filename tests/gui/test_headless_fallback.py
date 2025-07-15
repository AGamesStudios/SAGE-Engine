import importlib

import sage_engine.gui as gui


def test_headless_fallback(monkeypatch):
    monkeypatch.setattr(gui.metadata, "entry_points", lambda group=None: [])
    importlib.reload(gui)
    monkeypatch.setattr(gui, "import_module", lambda *a, **k: (_ for _ in ()).throw(ImportError()))
    backend = gui.load_backend()
    assert backend.__class__.__name__ == "HeadlessBackend"
