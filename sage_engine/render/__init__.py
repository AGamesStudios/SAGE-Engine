"""Backend-agnostic rendering facade."""
from __future__ import annotations

import os
from importlib import import_module
from typing import Any

from ..settings import settings

_backend = None


def _select_backend() -> str:
    backend = settings.render_backend or os.environ.get("SAGE_RENDER_BACKEND")
    return backend or "software"


def _load_backend(name: str):
    mod = import_module(f"sage_engine.render.backends.{name}")
    return mod.get_backend()


def init(output_target: Any = None) -> None:
    """Initialize the rendering system with selected backend."""
    global _backend
    if _backend is not None:
        return
    backend_name = _select_backend()
    _backend = _load_backend(backend_name)
    _backend.init(output_target)


def begin_frame() -> None:
    if _backend:
        _backend.begin_frame()


def draw_sprite(image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
    if _backend:
        _backend.draw_sprite(image, x, y, w, h, rotation)


def draw_rect(x: int, y: int, w: int, h: int, color: Any) -> None:
    if _backend:
        _backend.draw_rect(x, y, w, h, color)


def end_frame() -> None:
    if _backend:
        _backend.end_frame()


def shutdown() -> None:
    global _backend
    if _backend:
        _backend.shutdown()
        _backend = None


# helpers for tests

def _get_backend():
    return _backend
