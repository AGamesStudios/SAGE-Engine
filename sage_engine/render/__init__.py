"""Backend-agnostic rendering facade."""
from __future__ import annotations

import os
from importlib import import_module
from typing import Any

from ..settings import settings

_backend = None
_context = None
_viewport = None


def _select_backend() -> str:
    backend = settings.render_backend or os.environ.get("SAGE_RENDER_BACKEND")
    return backend or "software"


def _load_backend(name: str):
    mod = import_module(f"sage_engine.render.backends.{name}")
    return mod.get_backend()


def init(output_target: Any = None) -> None:
    """Initialize the rendering system with selected backend."""
    global _backend, _context
    if _backend is None:
        backend_name = _select_backend()
        _backend = _load_backend(backend_name)
    if _context is None:
        _context = _backend.create_context(output_target)


def begin_frame() -> None:
    if _context:
        _context.begin_frame()


def draw_sprite(image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
    if _context:
        _context.draw_sprite(image, x, y, w, h, rotation)


def draw_rect(x: int, y: int, w: int, h: int, color: Any) -> None:
    if _context:
        _context.draw_rect(x, y, w, h, color)


def end_frame() -> None:
    if _context:
        _context.end_frame()


def present(buffer: memoryview, handle: Any = None) -> None:
    if handle is None:
        ctx = _context
    else:
        ctx = _backend.create_context(handle)
    if ctx:
        ctx.present(buffer)

def create_context(handle: Any) -> Any:
    if _backend:
        return _backend.create_context(handle)
    return None

def set_viewport(width: int, height: int, preserve_aspect: bool = True, handle: Any = None) -> None:
    global _viewport
    if handle is None:
        ctx = _context
    else:
        ctx = _backend.create_context(handle)
    if ctx:
        ctx.set_viewport(0, 0, width, height)
    _viewport = (width, height, preserve_aspect)

def resize(width: int, height: int, handle: Any = None) -> None:
    if handle is None:
        ctx = _context
    else:
        ctx = _backend.create_context(handle)
    if ctx:
        ctx.resize(width, height)


def shutdown() -> None:
    global _backend, _context
    if _context:
        _context.shutdown()
        _context = None
    elif _backend:
        # ensure backend shutdown even if context failed
        _backend.shutdown()
    _backend = None


# helpers for tests

def _get_backend():
    return _backend

def _get_context():
    return _context

