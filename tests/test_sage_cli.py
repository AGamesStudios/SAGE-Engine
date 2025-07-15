import yaml
import tarfile
import json
import asyncio
import websockets
import sage

from sage_engine import bundles, adaptors
from sage_editor import hot_reload
import socketserver


class DummyServer:
    def __init__(self, *args, **kwargs):
        self.events = kwargs.get("events") if "events" in kwargs else {}
        if isinstance(self.events, dict):
            self.events.setdefault("addr", args[0] if args else ("", 0))

    def serve_forever(self):
        if isinstance(self.events, dict):
            self.events["run"] = True
        else:
            self.events.append("run")

    def shutdown(self):
        if isinstance(self.events, dict):
            self.events["shutdown"] = True
        else:
            self.events.append("shutdown")


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
    import http.server  # noqa: F401

    def server_factory(addr, handler):
        return DummyServer(addr, handler, events=events)

    async def fake_start():
        events["ws"] = True

    monkeypatch.setattr(socketserver, "TCPServer", server_factory)
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


def test_build_bundle_excludes_unused(monkeypatch, tmp_path):
    monkeypatch.chdir(tmp_path)
    pkg = tmp_path / "src" / "sage_adaptors"
    (pkg / "render").mkdir(parents=True)
    (pkg / "render" / "__init__.py").write_text("")
    (pkg / "audio").mkdir()
    (pkg / "audio" / "__init__.py").write_text("")
    (tmp_path / "assets").mkdir()

    monkeypatch.setattr(bundles, "load_bundle", lambda n: {"adaptors": {"list": ["render"]}})
    monkeypatch.setattr(adaptors, "load_adaptors", lambda names=None: None)

    assert sage.main(["build", "--bundle", "retro2d"]) == 0
    with tarfile.open(tmp_path / "build" / "game.fpk", "r:gz") as tf:
        names = tf.getnames()
    assert "src/sage_adaptors/render/__init__.py" in names
    assert "src/sage_adaptors/audio/__init__.py" not in names
    manifest = json.loads((tmp_path / "build" / "manifest.json").read_text())
    assert any(p.endswith("render/__init__.py") for p in manifest)


def test_serve_websocket(monkeypatch):
    events = []

    import http.server  # noqa: F401

    async def dummy_listener(host="localhost", port=8765):
        async def handler(ws):
            await ws.send("hello")
        server = await websockets.serve(handler, host, port)
        await asyncio.sleep(0.1)
        server.close()
        await server.wait_closed()
        events.append("done")

    monkeypatch.setattr(hot_reload, "start_listener", dummy_listener)
    monkeypatch.setattr(socketserver, "TCPServer", lambda *a, **k: DummyServer(*a, events=events, **k))

    assert sage.main(["serve", "--port", "0"]) == 0
    assert "done" in events
