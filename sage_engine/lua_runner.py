"""Execute Lua scripts using lupa."""
from __future__ import annotations

from pathlib import Path
from typing import Any

from lupa import LuaRuntime

_lua: LuaRuntime | None = None


def _get_lua() -> LuaRuntime:
    global _lua
    if _lua is None:
        _lua = LuaRuntime(unpack_returned_tuples=True)
    return _lua


def set_lua_globals(**values: Any) -> None:
    """Expose Python objects to Lua."""
    lua = _get_lua()
    for name, value in values.items():
        lua.globals()[name] = value


def run_lua_script(path: str) -> None:
    """Execute a Lua script file."""
    lua = _get_lua()
    source = Path(path).read_text(encoding="utf-8")
    lua.execute(source)


__all__ = ["run_lua_script", "set_lua_globals"]
