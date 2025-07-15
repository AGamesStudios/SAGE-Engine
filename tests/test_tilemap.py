from sage_engine.tilemap import TileLayer, autowang


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
