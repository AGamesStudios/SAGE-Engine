"""Wrapper for the optional lupa LuaJIT runtime."""

from __future__ import annotations

try:
    from lupa import LuaRuntime  # type: ignore
    AVAILABLE = True
except Exception:  # pragma: no cover - optional dependency
    AVAILABLE = False
    LuaRuntime = None  # type: ignore


def get_runtime() -> LuaRuntime | None:
    """Return a new :class:`LuaRuntime` instance or ``None`` if unavailable."""
    if AVAILABLE:
        try:
            return LuaRuntime()
        except Exception:  # pragma: no cover - runtime errors
            return None
    return None

__all__ = ["AVAILABLE", "get_runtime"]
