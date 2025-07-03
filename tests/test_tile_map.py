from engine.entities.tile_map import TileMap
from engine.formats import save_sagemap, load_sagemap


def test_sagemap_roundtrip(tmp_path):
    path = tmp_path / "map.sagemap"
    data = {
        "tileset": "tiles.png",
        "tile_width": 8,
        "tile_height": 8,
        "width": 2,
        "height": 2,
        "data": [1, 0, 0, 1],
    }
    save_sagemap(data, path)
    loaded = load_sagemap(path)
    assert loaded["tileset"] == "tiles.png"
    tm = TileMap(map_file=str(path))
    assert tm.width == 2 and tm.height == 2
    assert tm.data == [1, 0, 0, 1]

