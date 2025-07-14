"""Load Python or Lua patcher scripts."""

from __future__ import annotations

import importlib.util
from pathlib import Path
from typing import Callable, Any

from .extras import lua

__all__ = ["load_patcher", "Patcher"]


class Patcher:
    """Simple wrapper for a patcher function."""

    def __init__(self, func: Callable[[Any], None]) -> None:
        self.func = func

    def apply(self, tree: Any) -> None:
        self.func(tree)


def _load_python(path: Path) -> Patcher:
    spec = importlib.util.spec_from_file_location(path.stem, path)
    if spec and spec.loader:
        module = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)  # type: ignore[arg-type]
    else:  # pragma: no cover - invalid spec
        raise ImportError(f"cannot load {path}")
    func = getattr(module, "apply", None)
    if not callable(func):
        raise AttributeError(f"{path} must define an apply() function")
    return Patcher(func)


def _load_lua(path: Path) -> Patcher:
    if not lua.AVAILABLE:
        raise RuntimeError("Lua runtime not available")
    runtime = lua.get_runtime()
    if runtime is None:
        raise RuntimeError("Lua runtime not available")
    code = path.read_text(encoding="utf8")
    lua_func = runtime.execute(code)

    def wrapper(tree: Any) -> None:
        lua_func(tree)

    return Patcher(wrapper)


def load_patcher(path: str | Path) -> Patcher:
    """Load a patcher script and return a :class:`Patcher`."""
    p = Path(path)
    if p.suffix == ".py":
        return _load_python(p)
    if p.suffix == ".lua":
        return _load_lua(p)
    raise ValueError(f"unsupported patcher type: {p.suffix}")
