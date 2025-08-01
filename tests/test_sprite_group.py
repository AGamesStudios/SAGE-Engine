from sage_engine.sprite import group, sprite_batch, sprite
from sage_engine.format import sageimg


def test_sprite_group_draw(tmp_path):
    p = tmp_path / "s.sageimg"
    p.write_bytes(sageimg.encode(b"\x00\x00\x00\xff", 1, 1))
    spr = sprite.load(str(p))
    grp = group.SpriteGroup(x=10, y=10, scale=1.0)
    grp.add(spr)
    batch = sprite_batch.SpriteBatch()
    grp.draw(batch)
    assert batch.count == 1
