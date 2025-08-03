import pytest
from sage_engine.resource.loader import load_engine_cfg

def test_load_engine_cfg_valid(tmp_path):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text(
        "[SAGECFG]\nwindow_title=Demo\nscreen_width=320\nscreen_height=240\nrender_backend=vulkan",
        encoding="utf8",
    )
    data = load_engine_cfg(cfg)
    assert data["window_title"] == "Demo"
    assert data["screen_width"] == 320
    assert data["screen_height"] == 240
    assert "unknown" not in data
    assert data["render_backend"] == "vulkan"


def test_load_engine_cfg_missing(tmp_path):
    cfg = tmp_path / "engine.sagecfg"
    with pytest.raises(FileNotFoundError):
        load_engine_cfg(cfg)


def test_load_engine_cfg_invalid_header(tmp_path):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text("name=Demo", encoding="utf8")
    with pytest.raises(ValueError):
        load_engine_cfg(cfg)


def test_load_engine_cfg_migration_and_unknown(tmp_path):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text("[SAGECFG]\nmodules=['core']\nrender_backend=software\nfoo=1", encoding="utf8")
    data = load_engine_cfg(cfg)
    assert data["boot_modules"] == ['core']
    assert "foo" not in data


def test_core_boot_ignores_bad_cfg(tmp_path, monkeypatch):
    import importlib, sys

    bad = tmp_path / "bad.sagecfg"
    bad.write_text("name=Demo", encoding="utf8")
    monkeypatch.setenv("SAGE_ENGINE_CFG", str(bad))
    core = importlib.import_module("sage_engine.core")
    core.core_boot()
    core.core_shutdown()
    sys.modules.pop("sage_engine.core")
