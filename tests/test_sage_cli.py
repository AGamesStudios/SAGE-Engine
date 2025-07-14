import json
import sage


def test_sage_cli_commands(monkeypatch):
    called = []
    monkeypatch.setitem(sage._COMMANDS, "build", lambda a: called.append("build"))
    assert sage.main(["build"]) == 0
    assert called == ["build"]


def test_sage_cli_create(tmp_path):
    target = tmp_path / "game"
    assert sage.main(["create", str(target), "-t", "platformer"]) == 0
    assert (target / "project.sageproject").exists()


def test_sage_cli_create_hello(tmp_path):
    target = tmp_path / "hello"
    assert sage.main(["create", str(target), "-t", "hello_sprite_py"]) == 0
    assert (target / "hello_sprite.py").exists()


def test_sage_cli_profile(tmp_path):
    prof = tmp_path / "trace.json"
    assert sage.main(["--profile", str(prof), "serve"]) == 0
    data = json.loads(prof.read_text())
    assert len(data.get("traceEvents", [])) == 4
