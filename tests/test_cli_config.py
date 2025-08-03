from pathlib import Path
from sage_engine.devtools import cli


def test_config_get_set(tmp_path, capsys):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text("render_backend = \"software\"")
    cli.main(["config", "set", "render_backend", "vulkan", "--file", str(cfg)])
    cli.main(["config", "get", "render_backend", "--file", str(cfg)])
    out = capsys.readouterr().out.strip()
    assert out == "vulkan"


def test_check_config_cli(tmp_path, capsys):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text("[SAGECFG]\nfoo=1", encoding="utf8")
    cli.main(["check-config", "--file", str(cfg)])
    out = capsys.readouterr().out
    assert "Unknown keys" in out
