"""Backend-agnostic rendering facade."""
from __future__ import annotations

import os
from importlib import import_module
from typing import Any
import time

from ..settings import settings
from ..events import on
from ..logger import logger
from .. import core
from . import stats
from types import SimpleNamespace

_backend = None
_context = None
_viewport = None
_micro_culling = False
_delta_render = False
_raster_cache = False
_adaptive_repaint = False
_frame_budget_ms: int | None = None
_frame_start = 0.0
_chunking = False
_culling = False
_batching = False


def _select_backend() -> str:
    backend = settings.render_backend or os.environ.get("SAGE_RENDER_BACKEND")
    return backend or "software"


def _load_backend(name: str):
    mod = import_module(f"sage_engine.render.backends.{name}")
    return mod.get_backend()


def enable_micro_culling(enabled: bool = True) -> None:
    """Toggle predictive micro culling."""
    global _micro_culling
    _micro_culling = enabled


def enable_delta_render(enabled: bool = True) -> None:
    """Toggle delta-render optimization."""
    global _delta_render
    _delta_render = enabled


def enable_raster_cache(enabled: bool = True) -> None:
    """Toggle raster cache manager."""
    global _raster_cache
    _raster_cache = enabled


def enable_adaptive_repaint(enabled: bool = True) -> None:
    """Toggle adaptive repaint scheduler."""
    global _adaptive_repaint
    _adaptive_repaint = enabled


def set_frame_budget(ms: int | None) -> None:
    """Set the maximum frame time in milliseconds."""
    global _frame_budget_ms
    _frame_budget_ms = ms


def enable_chunking(enabled: bool = True) -> None:
    global _chunking
    _chunking = enabled


def enable_culling(enabled: bool = True) -> None:
    global _culling
    _culling = enabled


def enable_batching(enabled: bool = True) -> None:
    global _batching
    _batching = enabled


def init(output_target: Any = None) -> None:
    """Initialize the rendering system with selected backend."""
    global _backend, _context
    if _backend is None:
        backend_name = _select_backend()
        _backend = _load_backend(backend_name)
    if _context is None:
        _context = _backend.create_context(output_target)


def begin_frame() -> None:
    global _frame_start
    _frame_start = time.perf_counter()
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
        ctx = _backend.create_context(handle) if _backend else None
    if ctx:
        ctx.present(buffer)
    if _frame_budget_ms is not None:
        elapsed = (time.perf_counter() - _frame_start) * 1000.0
        if elapsed > _frame_budget_ms:
            logger.warn(
                "[render] Frame budget exceeded: %.2fms > %dms",
                elapsed,
                _frame_budget_ms,
            )

def create_context(handle: Any) -> Any:
    if _backend:
        return _backend.create_context(handle)
    return None

def set_viewport(width: int, height: int, preserve_aspect: bool = True, handle: Any = None) -> None:
    global _viewport
    if handle is None:
        ctx = _context
    else:
        ctx = _backend.create_context(handle) if _backend else None
    if ctx:
        ctx.set_viewport(0, 0, width, height)
    _viewport = (width, height, preserve_aspect)

def resize(width: int, height: int, handle: Any = None) -> None:
    if handle is None:
        ctx = _context
    else:
        ctx = _backend.create_context(handle) if _backend else None
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


on("window_resized", lambda w, h: resize(w, h))

core.expose(
    "render",
    SimpleNamespace(
        init=init,
        begin_frame=begin_frame,
        end_frame=end_frame,
        present=present,
        create_context=create_context,
        resize=resize,
        _get_backend=_get_backend,
        _get_context=_get_context,
        enable_micro_culling=enable_micro_culling,
        enable_delta_render=enable_delta_render,
        enable_raster_cache=enable_raster_cache,
        enable_adaptive_repaint=enable_adaptive_repaint,
        set_frame_budget=set_frame_budget,
        enable_chunking=enable_chunking,
        enable_culling=enable_culling,
        enable_batching=enable_batching,
        shutdown=shutdown,
        stats=stats.stats,
    ),
)

