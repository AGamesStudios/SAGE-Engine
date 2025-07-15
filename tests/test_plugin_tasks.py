import asyncio
import types
from pathlib import Path

from sage_engine.plugins import (
    PluginManager,
    wait_plugin_tasks,
    cancel_plugin_tasks,
    _PENDING,
)


def _write_plugin(path: Path, content: str) -> None:
    path.write_text(content)


def test_async_plugin_tasks_wait(tmp_path):
    plugin = tmp_path / "p_async.py"
    _write_plugin(
        plugin,
        "import asyncio\nasync def init_engine(engine):\n    await asyncio.sleep(0)\n    engine.done = True\n",
    )
    mgr = PluginManager("engine", plugin_dir=str(tmp_path))
    engine = types.SimpleNamespace()

    async def run() -> None:
        mgr.load(engine)
        await wait_plugin_tasks(asyncio.get_running_loop())
        assert not _PENDING

    asyncio.run(run())
    assert getattr(engine, "done", False)


def test_async_plugin_tasks_cancel(tmp_path):
    plugin = tmp_path / "p_cancel.py"
    _write_plugin(
        plugin,
        "import asyncio\nasync def init_engine(engine):\n    await asyncio.sleep(1)\n",
    )
    mgr = PluginManager("engine", plugin_dir=str(tmp_path))

    async def run() -> None:
        mgr.load(object())
        cancel_plugin_tasks(asyncio.get_running_loop())
        assert not _PENDING

    asyncio.run(run())

