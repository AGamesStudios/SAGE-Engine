from pathlib import Path
from sage_engine.sprite import text
from sage_engine.sprite.font_atlas import FontTextureAtlas
from sage_engine.render import stats
from sage_engine.format import sageimg


def test_draw_text_updates_stats(tmp_path: Path, monkeypatch):
    img = tmp_path / "font.sageimg"
    img.write_bytes(sageimg.encode(b"\x00" * 4, 1, 1))
    (tmp_path / "font.sageimg.meta").write_text('{"A": [0,0,1,1]}')
    monkeypatch.setattr(text.gfx, "draw_text", lambda *a, **k: None)
    font = text.load_font(str(img), 12)
    stats.stats["text_glyphs_rendered"] = 0
    text.draw_text("AA", 0, 0, font)
    assert stats.stats["text_glyphs_rendered"] == 2

