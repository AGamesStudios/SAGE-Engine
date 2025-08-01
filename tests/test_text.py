from pathlib import Path
from sage_engine.sprite import text, sprite, draw
from sage_engine import render, gfx
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


def test_text_and_sprite_scene(tmp_path: Path, monkeypatch):
    img = tmp_path / "sprite.sageimg"
    img.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    font_img = tmp_path / "font.sageimg"
    font_img.write_bytes(sageimg.encode(b"\x00" * 4, 1, 1))
    (tmp_path / "font.sageimg.meta").write_text('{"A": [0,0,1,1]}')
    monkeypatch.setattr(text.gfx, "draw_text", lambda *a, **k: None)
    gfx.init(1, 1)
    render.init(None)
    gfx.begin_frame()
    spr = sprite.load(str(img))
    font = text.load_font(str(font_img), 12)
    draw.sprite(spr, 0, 0)
    text.draw_text("A", 0, 0, font)
    draw.flush()
    assert stats.stats["sprites_drawn"] >= 1
    assert stats.stats["text_glyphs_rendered"] >= 1
    gfx.shutdown()
    render.shutdown()

