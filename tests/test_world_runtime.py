from sage_engine.world.parser import parse_world_file
from sage_engine.world.runtime import WorldRuntime


def test_parse_world(tmp_path):
    cfg_path = tmp_path / "level.sagecfg"
    cfg_path.write_text(
        """world:\n  name: test\n  objects:\n    - type: Player\n      position: [0, 0]\n  on_start: []\n""",
        encoding="utf8",
    )
    cfg = parse_world_file(cfg_path)
    assert cfg.name == "test"
    assert len(cfg.objects) == 1


def test_runtime_load(tmp_path):
    cfg_path = tmp_path / "level.sagecfg"
    cfg_path.write_text(
        """world:\n  objects:\n    - type: Player\n      position: [1, 2]\n""",
        encoding="utf8",
    )
    rt = WorldRuntime()
    rt.load_scene("lvl", cfg_path)
    rt.set_scene("lvl")
    assert len(rt.objects) == 1
    obj = rt.objects[0]
    assert obj.position.x == 1
    assert obj.position.y == 2


def test_switch_layer():
    rt = WorldRuntime()
    rt.switch_layer("ui")
    assert rt.active_layer == "ui"
