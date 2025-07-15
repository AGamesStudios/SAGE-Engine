import asyncio
import types
from sage_engine.editor import hot_reload


class DummyWS:
    def __init__(self, messages):
        self._msgs = list(messages)

    async def send(self, msg):
        pass

    def __aiter__(self):
        return self

    async def __anext__(self):
        if not self._msgs:
            raise StopAsyncIteration
        return self._msgs.pop(0)


async def run_handler(ws):
    await hot_reload._handle(ws)


def test_handle_reload(monkeypatch):
    called = {}

    def fake_import(name):
        called["name"] = name
        return types.SimpleNamespace()

    def fake_reload(mod):
        called["reloaded"] = True

    monkeypatch.setattr(hot_reload.importlib, "import_module", fake_import)
    monkeypatch.setattr(hot_reload.importlib, "reload", fake_reload)
    monkeypatch.setattr(hot_reload.adaptors, "load_adaptors", lambda names: called.setdefault("load", names))
    ws = DummyWS(["reload sage_engine.adaptors.render"])
    asyncio.run(run_handler(ws))
    assert called == {"name": "sage_engine.adaptors.render", "reloaded": True, "load": ["render"]}


def test_handle_toast():
    ws = DummyWS(["toast hello"])
    asyncio.run(run_handler(ws))

