import importlib

import sage_engine.render as render


def test_null_fallback(monkeypatch):
    monkeypatch.setattr(render.metadata, "entry_points", lambda group=None: [])
    importlib.reload(render)
    monkeypatch.setattr(render, "import_module", lambda *a, **k: (_ for _ in ()).throw(ImportError()))
    backend = render.load_backend()
    assert backend.__class__.__name__ == "HeadlessBackend"
