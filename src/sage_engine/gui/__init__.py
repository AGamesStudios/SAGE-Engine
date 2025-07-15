"""Pluggable GUI backend loader."""

from __future__ import annotations

import os
import warnings
from importlib import import_module, metadata
from typing import Optional

from .base import GuiBackend
from .headless import HeadlessBackend

_BACKEND: Optional[GuiBackend] = None


def _load_entry(name: str) -> Optional[GuiBackend]:
    for ep in metadata.entry_points(group="sage_gui"):
        if ep.name == name:
            try:
                backend_cls = ep.load()
                return backend_cls()
            except Exception as exc:
                warnings.warn(f"Failed to load GUI backend '{name}': {exc}")
                return None
    # fallback for editable installs
    try:
        module = import_module(f"gui_{name}.backend")
        for attr in dir(module):
            if attr.endswith("Backend") and attr != "GuiBackend":
                backend_cls = getattr(module, attr)
                return backend_cls()
    except Exception:
        return None
    return None


def load_backend(name: str = "auto") -> GuiBackend:
    """Load a GUI backend by name and announce the result."""
    if name != "auto":
        backend = _load_entry(name)
        if backend:
            print(f"GUI backend: {name}")
            return backend
    else:
        for candidate in ("qt6", "qt5", "tk"):
            backend = _load_entry(candidate)
            if backend:
                print(f"GUI backend: {candidate}")
                return backend
    warnings.warn("No GUI backend available, falling back to headless")
    print("GUI backend: headless")
    return HeadlessBackend()


def get_backend(name: Optional[str] = None) -> GuiBackend:
    global _BACKEND
    if name is None:
        name = os.environ.get("SAGE_GUI", "auto")
    if _BACKEND is None or name != "auto":
        _BACKEND = load_backend(name)
    return _BACKEND

__all__ = ["GuiBackend", "GuiEvent", "get_backend", "load_backend"]

from .base import GuiEvent  # noqa: E402  (after __all__)
