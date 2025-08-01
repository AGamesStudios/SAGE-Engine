from sage_engine.sprite import sprite_batch, sprite, draw
from sage_engine.texture import Texture
from sage_engine import gfx


def test_sprite_performance():
    tex = Texture(width=1, height=1, pixels=b"\x00\x00\x00\xff")
    spr = sprite.Sprite(tex, (0, 0, 1, 1))
    batch = sprite_batch.SpriteBatch(10000)
    for i in range(10000):
        batch.add(spr, 0, 0, 1, 1)
    gfx.init(32, 32)
    gfx.begin_frame()
    frame = draw.draw_batch(batch)
    assert len(frame) == 32 * 32 * 4
    gfx.shutdown()
