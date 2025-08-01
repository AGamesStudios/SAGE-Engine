import os


def test_default_font_auto_loaded(monkeypatch):
    from sage_engine import gfx
    from sage_engine.gui import style
    calls = []
    monkeypatch.setattr(gfx, "load_font", lambda path, size: calls.append((path, size)))
    from sage_engine.gui.manager import GUIManager
    GUIManager()
    assert (style.DEFAULT_FONT_PATH, 14) in calls


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


def test_default_font_loading():
    from sage_engine import gfx
    from sage_engine.gui import style
    path = style.DEFAULT_FONT_PATH
    assert os.path.exists(path)
    font = gfx.load_font(path, 14)
    assert font is not None
