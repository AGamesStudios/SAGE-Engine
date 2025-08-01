from sage_engine.sprite import sprite_batch, sprite, draw
from sage_engine.graphic import api


def test_sprite_performance():
    spr = sprite.Sprite(1, 1, b"\x00\x00\x00\xff")
    batch = sprite_batch.SpriteBatch(10000)
    for i in range(10000):
        batch.add(spr, 0, 0, 1, 1)
    api.init(32, 32)
    frame = draw.draw_batch(batch)
    assert len(frame) == 32 * 32 * 4
