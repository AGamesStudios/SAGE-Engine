"""Load Python or Lua patcher scripts with hot reload."""

from __future__ import annotations

import importlib.util
import logging
from pathlib import Path
from typing import Callable, Any

from .extras import lua

logger = logging.getLogger(__name__)

__all__ = ["load_patcher", "Patcher"]


class Patcher:
    """Wrapper for a patcher function that reloads on file change."""

    def __init__(
        self,
        func: Callable[[Any], None],
        *,
        path: Path | None = None,
        loader: Callable[[Path], Callable[[Any], None]] | None = None,
    ) -> None:
        self.func = func
        self._path = path
        self._loader = loader
        self._mtime = path.stat().st_mtime if path else 0.0

    def apply(self, tree: Any) -> None:
        if self._path and self._loader:
            mtime = self._path.stat().st_mtime
            if mtime != self._mtime:
                self.func = self._loader(self._path)
                self._mtime = mtime
        self.func(tree)


def _load_python_func(path: Path) -> Callable[[Any], None]:
    spec = importlib.util.spec_from_file_location(path.stem, path)
    if spec and spec.loader:
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)  # type: ignore[arg-type]
    else:  # pragma: no cover - invalid spec
        raise ImportError(f"cannot load {path}")
    func = getattr(module, "apply", None)
    if not callable(func):
        raise AttributeError(f"{path} must define an apply() function")
    return func


def _load_lua_func(path: Path) -> Callable[[Any], None]:
    if not lua.AVAILABLE:
        logger.warning("Lua runtime not available; skipping %s", path)
        return lambda _tree: None
    runtime = lua.get_runtime()
    if runtime is None:
        logger.warning("Lua runtime not available; skipping %s", path)
        return lambda _tree: None
    code = path.read_text(encoding="utf8")
    lua_func = runtime.execute(code)

    def wrapper(tree: Any) -> None:
        lua_func(tree)

    return wrapper


def load_patcher(path: str | Path) -> Patcher:
    """Load a patcher script and return a :class:`Patcher`."""
    p = Path(path)
    if p.suffix == ".py":
        func = _load_python_func(p)
        return Patcher(func, path=p, loader=_load_python_func)
    if p.suffix == ".lua":
        func = _load_lua_func(p)
        return Patcher(func, path=p, loader=_load_lua_func)
    raise ValueError(f"unsupported patcher type: {p.suffix}")
