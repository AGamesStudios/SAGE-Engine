from pathlib import Path
from sage_engine.sprite import sprite, draw
from sage_engine.graphic import api
from sage_engine.format import sageimg


def test_load_and_cache(tmp_path: Path):
    p = tmp_path / "img.sageimg"
    p.write_bytes(sageimg.encode(b"\x00", 1, 1))
    s1 = sprite.load(str(p))
    s2 = sprite.load(str(p))
    assert s1 is s2
    assert (s1.width, s1.height) == (1, 1)


def test_draw_and_flush(tmp_path: Path):
    p = tmp_path / "img.sageimg"
    p.write_bytes(sageimg.encode(b"\x00", 1, 1))
    s = sprite.load(str(p))
    api.init(2, 2)
    draw.sprite(s, 0, 0)
    buf = draw.flush()
    assert len(buf) == 4 * 2 * 2

