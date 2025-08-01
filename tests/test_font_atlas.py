from pathlib import Path

from sage_engine.sprite.font_atlas import FontTextureAtlas
from sage_engine.texture import TextureCache
from sage_engine.format import sageimg
from sage_engine import gfx


def test_font_atlas_load(monkeypatch, tmp_path: Path):
    img = tmp_path / "font.sageimg"
    img.write_bytes(sageimg.encode(b"\x00" * 4, 1, 1))
    (tmp_path / "font.sageimg.meta").write_text('{"A": [0, 0, 1, 1]}')
    monkeypatch.setattr(gfx, "load_font", lambda p, size: object())
    fa = FontTextureAtlas(str(img), 12)
    fa.draw_text("A", 0, 0)
