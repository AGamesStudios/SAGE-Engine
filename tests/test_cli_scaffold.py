from pathlib import Path
from sage_engine.devtools import cli


def test_new_project(tmp_path):
    dest = tmp_path / "Game"
    cli.new_project(str(dest), "blank-2d")
    assert (dest / "engine.sagecfg").exists()
    assert (dest / "main.py").exists()


def test_run_project(tmp_path):
    dest = tmp_path / "Game"
    cli.new_project(str(dest), "blank-2d")
    cli.run_project(dest)


def test_init_project(tmp_path):
    cli.init_engine_cfg(tmp_path)
    assert (tmp_path / "engine.sagecfg").exists()
    assert (tmp_path / "world").is_dir()

