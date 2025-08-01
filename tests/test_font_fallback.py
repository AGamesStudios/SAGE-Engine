from sage_engine.sprite import text
from sage_engine.logger import logger


def test_font_missing(monkeypatch):
    msgs = []
    monkeypatch.setattr(logger, "warning", lambda m, *a, **k: msgs.append(m))
    font = text.load_font("missing.ttf", 12)
    text.draw_text("hi", 0, 0, font)
    assert any("Font not found" in m for m in msgs)

