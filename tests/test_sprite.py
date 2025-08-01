from pathlib import Path
from sage_engine.sprite import sprite, draw
from sage_engine import gfx, render
from sage_engine.format import sageimg


def test_load_and_cache(tmp_path: Path):
    p = tmp_path / "img.sageimg"
    p.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    s1 = sprite.load(str(p))
    s2 = sprite.load(str(p))
    assert s1 is s2
    assert s1.texture.get_size() == (1, 1)


def test_draw_and_flush(tmp_path: Path):
    p = tmp_path / "img.sageimg"
    p.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    s = sprite.load(str(p))
    gfx.init(2, 2)
    render.init(None)
    gfx.begin_frame()
    draw.sprite(s, 0, 0)
    buf = draw.flush()
    assert len(buf) == 4 * 2 * 2
    gfx.shutdown()
    render.shutdown()


def test_png_rejected(tmp_path, monkeypatch):
    p = tmp_path / "bad.png"
    p.write_bytes(b"fake")
    from sage_engine.texture import Texture
    from sage_engine.logger import logger
    msgs = []
    monkeypatch.setattr(logger, "error", lambda m, *a, **k: msgs.append(m))
    tex = Texture()
    tex.load(str(p))
    assert any("external image formats" in m for m in msgs)


def test_texture_cache_usage(tmp_path: Path):
    p = tmp_path / "tex.sageimg"
    p.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    from sage_engine.texture import TextureCache
    t1 = TextureCache.load(str(p))
    t2 = TextureCache.load(str(p))
    assert t1 is t2


def test_render_stats(tmp_path: Path):
    p = tmp_path / "img.sageimg"
    p.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    s = sprite.load(str(p))
    gfx.init(1, 1)
    render.init(None)
    gfx.begin_frame()
    draw.sprite(s, 0, 0)
    draw.flush()
    assert render.stats.stats["sprites_drawn"] >= 1
    assert render.stats.stats["textures_bound"] == 1
    gfx.shutdown()
    render.shutdown()

