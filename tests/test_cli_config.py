from pathlib import Path
from sage_engine.devtools import cli


def test_config_get_set(tmp_path, capsys):
    cfg = tmp_path / "engine.sagecfg"
    cfg.write_text("render_backend = \"software\"")
    cli.main(["config", "set", "render_backend", "vulkan", "--file", str(cfg)])
    cli.main(["config", "get", "render_backend", "--file", str(cfg)])
    out = capsys.readouterr().out.strip()
    assert out == "vulkan"
