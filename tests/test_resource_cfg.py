from sage_engine.resource.loader import load_cfg


def test_load_cfg_simple(tmp_path):
    cfg = tmp_path / "game.sagecfg"
    cfg.write_text('name = "Demo"\nwidth = 320', encoding="utf8")
    data = load_cfg(cfg)
    assert data["name"] == "Demo"
    assert data["width"] == 320


def test_load_cfg_yaml(tmp_path):
    cfg = tmp_path / "data.sagecfg"
    cfg.write_text("name: demo\nvalue: 5", encoding="utf8")
    data = load_cfg(cfg)
    assert data["name"] == "demo"
    assert data["value"] == 5

