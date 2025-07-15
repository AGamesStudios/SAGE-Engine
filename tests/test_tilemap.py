from sage_engine.tilemap import TileLayer, autowang
from sage_engine import physics


def test_load_layer(tmp_path):
    tmx = tmp_path / "map.tmx"
    tmx.write_text(
        """<?xml version='1.0' encoding='UTF-8'?>
<map width='2' height='2' tilewidth='32' tileheight='32'>
  <layer name='Layer 1' width='2' height='2'>
    <data encoding='csv'>1,0,0,1</data>
  </layer>
</map>
"""
    )
    layer = TileLayer.from_tmx(str(tmx))
    assert layer.width == 2 and layer.height == 2
    assert layer.tiles == [1, 0, 0, 1]


def test_autowang():
    layer = TileLayer(3, 3, [1] * 9)
    autowang(layer)
    center = layer.tiles[4]
    assert center == 16  # 0b1111 + offset


def test_parallax_and_collision(tmp_path):
    tmx = tmp_path / "map.tmx"
    tmx.write_text(
        """<?xml version='1.0' encoding='UTF-8'?>
<map width='1' height='1' tilewidth='32' tileheight='32'>
  <tileset firstgid='1'>
    <tile id='0'>
      <properties><property name='collidable' value='true'/></properties>
    </tile>
  </tileset>
  <layer name='base' width='1' height='1'>
    <data encoding='csv'>0</data>
  </layer>
  <layer name='solid' width='1' height='1'>
    <data encoding='csv'>1</data>
  </layer>
  <layer name='par' width='1' height='1'>
    <properties><property name='parallax' value='0.5'/></properties>
    <data encoding='csv'>0</data>
  </layer>
</map>
"""
    )

    world = physics.World()
    TileLayer.from_tmx(str(tmx), layer=1, world=world)
    par_layer = TileLayer.from_tmx(str(tmx), layer=2)
    off = par_layer.draw_offset(100, 50)
    assert off == (50.0, 25.0)
    world.step(0.1)
    static = [b for b in world.bodies if b.behaviour == "static"]
    assert static and static[0].x == 0 and static[0].y == 0
