import yaml
import sage


def test_sage_cli_commands(monkeypatch):
    called = []
    monkeypatch.setitem(sage._COMMANDS, "build", lambda a: called.append("build"))
    assert sage.main(["build"]) == 0
    assert called == ["build"]


def test_build_creates_package(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    (tmp_path / "assets").mkdir()
    assert sage.main(["build"]) == 0
    assert (tmp_path / "build" / "game.fpk").exists()
    assert (tmp_path / "build" / "game.exe").exists()
    assert (tmp_path / "build" / "game.wasm").exists()


def test_featherize_creates_cache(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    py = tmp_path / "script.py"
    py.write_text("print('hi')")
    lua = tmp_path / "script.lua"
    lua.write_text("print('hi')")
    assert sage.main(["featherize"]) == 0
    cache = tmp_path / "build" / "cache"
    assert (cache / "script.mpy").exists()
    assert (cache / "script.luac").exists()


def test_serve_starts_server(monkeypatch):
    events = {}

    class DummyServer:
        def __init__(self, addr, handler):
            events["addr"] = addr

        def serve_forever(self):
            events["run"] = True

        def shutdown(self):
            events["shutdown"] = True

    async def fake_start():
        events["ws"] = True

    monkeypatch.setattr("socketserver.TCPServer", DummyServer)
    monkeypatch.setattr("sage_editor.hot_reload.start_listener", fake_start)
    assert sage.main(["serve", "--port", "0"]) == 0
    assert events == {"addr": ("0.0.0.0", 0), "run": True, "ws": True, "shutdown": True}


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
    data = yaml.safe_load(prof.read_text())
    assert len(data.get("traceEvents", [])) == 4


def test_sage_cli_migrate(tmp_path):
    proj = tmp_path / "proj.sageproject"
    proj.write_text(yaml.safe_dump({"scene": "x.sagescene"}))
    assert sage.main(["migrate", str(tmp_path)]) == 0
    data = yaml.safe_load(proj.read_text())
    assert data["scene_file"] == "x.sagescene"
    assert data["version"] == "0.0.1"
