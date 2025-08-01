from sage_engine.gui.manager import GUIManager


def test_gui_custom_fallback(monkeypatch):
    called = []
    monkeypatch.setattr('sage_engine.gfx.load_font', lambda p, s: None)
    mgr = GUIManager(fallback_fonts=["missing.ttf"])
    assert mgr._default_font is None
