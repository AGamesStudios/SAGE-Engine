from sage_engine.sprite import sprite_batch, sprite
from sage_engine.format import sageimg


def _dummy_sprite() -> sprite.Sprite:
    return sprite.Sprite(1, 1, b"\x00\x00\x00\xff")

def test_batch_add():
    spr = _dummy_sprite()
    batch = sprite_batch.SpriteBatch(2)
    batch.add(spr, 1, 2, 3, 4, (255, 255, 255, 255))
    batch.add(spr, 5, 6, 7, 8, (255, 255, 0, 0))
    assert batch.count == 2
    assert batch.x[1] == 5
    batch.add(spr, 9, 9, 9, 9, (0, 0, 0, 0))
    assert batch.count == 2  # capacity limit
