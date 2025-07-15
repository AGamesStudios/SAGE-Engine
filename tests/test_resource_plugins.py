from sage_engine.formats import load_resource, save_resource

def test_load_save_resource(tmp_path):
    path = tmp_path / "map.sagemap"
    data = {
        "tileset": "t.png",
        "tile_width": 8,
        "tile_height": 8,
        "width": 1,
        "height": 1,
        "data": [0],
    }
    save_resource(data, path)
    loaded = load_resource(path)
    assert loaded["width"] == 1 and loaded["height"] == 1
