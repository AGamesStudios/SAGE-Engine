from pathlib import Path

from sage_engine.sprite import sprite_batch, sprite, draw
from sage_engine.texture import Texture


def _dummy_sprite() -> sprite.Sprite:
    tex = Texture(width=1, height=1, pixels=b"\x00\x00\x00\xff")
    return sprite.Sprite(tex, (0, 0, 1, 1))


def test_batch_draw(tmp_path: Path):
    spr = _dummy_sprite()
    batch = sprite_batch.SpriteBatch(4)
    for i in range(3):
        batch.add(spr, i, i, 1, 1)
    from sage_engine import gfx, render
    gfx.init(4, 4)
    render.init(None)
    gfx.begin_frame()
    frame = draw.draw_batch(batch)
    assert len(frame) == 4 * 4 * 4
    gfx.shutdown()
    render.shutdown()
