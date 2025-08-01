def test_default_font_auto_loaded(monkeypatch):
    from sage_engine import gfx
    calls = []
    monkeypatch.setattr(gfx, "load_font", lambda path, size: calls.append((path, size)))
    from sage_engine.gui.manager import GUIManager
    GUIManager()
    assert ("resources/fonts/default.ttf", 14) in calls


def test_font_missing_logs(monkeypatch, caplog):
    from sage_engine.gui.widgets import Button
    from sage_engine.logger import logger
    import os
    orig_exists = os.path.exists
    monkeypatch.setattr(os.path, "exists", lambda p: False if p.endswith("default.ttf") else orig_exists(p))
    msgs = []
    monkeypatch.setattr(logger, "warning", lambda m, *a, **k: msgs.append(m))
    btn = Button(text="t")
    btn.draw()
    assert any("Font not found" in m for m in msgs)
