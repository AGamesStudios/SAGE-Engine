"""Watch script files for changes and reload them."""
from __future__ import annotations

import threading
import time
from pathlib import Path
from typing import Dict

from sage_fs import FlowRunner

from .lua_runner import run_lua_script
from .python_runner import run_python_script


class ScriptsWatcher:
    def __init__(
        self,
        folder: str = "data/scripts",
        flow_runner: FlowRunner | None = None,
        lua_runner=run_lua_script,
        python_runner=run_python_script,
    ) -> None:
        self.folder = Path(folder)
        self.flow_runner = flow_runner or FlowRunner()
        self.lua_runner = lua_runner
        self.python_runner = python_runner
        self._mtimes: Dict[Path, float] = {}
        self._stop = False
        self._thread: threading.Thread | None = None

    def scan(self) -> None:
        if not self.folder.is_dir():
            return
        for path in self.folder.iterdir():
            if path.suffix not in {".lua", ".sage_fs", ".py"}:
                continue
            mtime = path.stat().st_mtime
            if self._mtimes.get(path) != mtime:
                self._mtimes[path] = mtime
                if path.suffix == ".lua":
                    self.lua_runner(str(path))
                elif path.suffix == ".py":
                    self.python_runner(str(path))
                else:
                    self.flow_runner.run_file(str(path))

    def watch(self, interval: float = 1.0) -> None:
        while not self._stop:
            self.scan()
            time.sleep(interval)

    def start(self, interval: float = 1.0) -> None:
        if self._thread:
            return
        self._stop = False
        self._thread = threading.Thread(target=self.watch, args=(interval,), daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._stop = True
        if self._thread:
            self._thread.join(timeout=0.1)
            self._thread = None


__all__ = ["ScriptsWatcher"]
