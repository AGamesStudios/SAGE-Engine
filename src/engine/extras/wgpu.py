"""Wrapper for the optional wgpu dependency."""

from __future__ import annotations
import importlib

try:
    wgpu = importlib.import_module("wgpu")  # type: ignore
    from wgpu import utils as _utils  # type: ignore

    AVAILABLE = wgpu is not None
except Exception:  # pragma: no cover - optional dependency
    AVAILABLE = False
    wgpu = None  # type: ignore
    _utils = None  # type: ignore


def get_default_device():
    """Return the default WGPU device or ``None`` if unavailable."""
    if AVAILABLE and _utils is not None:
        try:
            return _utils.get_default_device()
        except Exception:  # pragma: no cover - runtime errors
            return None
    return None


__all__ = ["AVAILABLE", "wgpu", "get_default_device"]
