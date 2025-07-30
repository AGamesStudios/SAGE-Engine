import json
from sage_engine import world


def test_world_load_migration(tmp_path):
    p = tmp_path / "scene.json"
    data = {"engine_version": 1, "entities": [{"role": "sprite"}]}
    p.write_text(json.dumps(data))
    objs = world.load(p)
    assert objs and objs[0]["role"] == "sprite"
