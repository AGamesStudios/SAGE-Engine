from sage_engine.resource.loader import load_engine_cfg

def test_load_engine_cfg_valid(tmp_path):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text(
        "name=Demo\nschema_version=1\nwidth=320\nheight=240\nrender_backend=vulkan\nunknown=42",
        encoding="utf8",
    )
    data = load_engine_cfg(cfg)
    assert data["name"] == "Demo"
    assert data["schema_version"] == 2
    assert "unknown" not in data
    assert data["render_backend"] == "vulkan"
