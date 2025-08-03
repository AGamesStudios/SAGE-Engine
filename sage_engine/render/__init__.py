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
from ..transform import Camera2D
from ..graphics.camera3d import Camera3D
from ..graphics.math3d import Matrix4, Vector3
from .units.render3d import Render3DUnit
from types import SimpleNamespace

_backend = None
_context = None
_viewport = None
_camera: Camera2D | None = None
_micro_culling = False
_delta_render = False
_raster_cache = False
_adaptive_repaint = False
_frame_budget_ms: int | None = None
_frame_start_ns = 0
_chunking = False
_culling = False
_batching = False
_render3d: Render3DUnit | None = None


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


def set_camera3d(camera: Camera3D) -> None:
    """Set the active 3D camera used for mesh drawing."""
    cam_api = core.get("camera3d")
    if cam_api is None:
        from ..camera import runtime as _cam_rt  # noqa: F401
        cam_api = core.get("camera3d")
    if cam_api and "set_active_camera" in cam_api:
        cam_api["set_active_camera"](camera)


def _ensure_render3d() -> Render3DUnit:
    global _render3d
    if _render3d is None:
        from ..gfx import _runtime as gfx_runtime  # type: ignore
        _render3d = Render3DUnit(gfx_runtime)
    return _render3d


def draw_mesh(mesh, model: Matrix4 | None = None, color=(255, 255, 255)) -> None:
    unit = _ensure_render3d()
    unit.draw_mesh(mesh, model or Matrix4.identity(), color)


def set_camera(camera: Camera2D) -> None:
    """Set the active camera used for visibility checks."""
    global _camera
    _camera = camera


def init(output_target: Any = None) -> None:
    """Initialize the rendering system with selected backend."""
    global _backend, _context
    if _backend is None:
        backend_name = _select_backend()
        _backend = _load_backend(backend_name)
    if _context is None:
        _context = _backend.create_context(output_target)


def begin_frame() -> None:
    global _frame_start_ns
    _frame_start_ns = time.perf_counter_ns()
    stats.stats["frame_id"] += 1
    logger.debug("[render] Starting frame...", tag="render")
    tr = core.get("transform_runtime")
    if _camera is None:
        logger.error("[render] No active camera found for frame render.")
    elif tr and "prepare" in tr and "visible" in tr:
        try:
            tr["prepare"]()
            tr["visible"](_camera)
        except Exception:
            pass
    if _context:
        _context.begin_frame()


def draw_sprite(image: Any, x: int, y: int, w: int, h: int, rotation: float = 0.0) -> None:
    if _context:
        logger.debug(
            "draw_sprite tex=%s pos=%d,%d size=%dx%d",
            getattr(image, "id", id(image)),
            x,
            y,
            w,
            h,
            tag="render",
        )
        _context.draw_sprite(image, x, y, w, h, rotation)
    else:
        logger.debug("draw_sprite skipped; no context", tag="render")


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
    elapsed = (time.perf_counter_ns() - _frame_start_ns) / 1_000_000.0
    stats.stats["frame_ms"] = elapsed
    stats.stats["ms_frame"] = elapsed
    stats.stats["frame_time"] = elapsed
    prev_avg = stats.stats.get("frame_time_avg", 0.0)
    stats.stats["frame_time_avg"] = prev_avg * 0.9 + elapsed * 0.1 if prev_avg else elapsed
    stats.update_fps(elapsed)
    avg = stats.stats.get("fps_avg", 0.0)
    if avg > 0:
        avg_ms = 1000.0 / avg
        if elapsed > avg_ms * 2:
            logger.warn(
                "[render] Frame time spike: %.2fms (avg %.2fms)",
                elapsed,
                avg_ms,
                tag="render",
            )
    if elapsed > 5000.0:
        logger.error("[render] runaway frame detected: %.2fms", elapsed, tag="render")
    elif elapsed > 5.0:
        logger.warn("[render] Slow frame: %.2fms", elapsed, tag="render")
    if _frame_budget_ms is not None:
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
        set_camera3d=set_camera3d,
        draw_mesh=draw_mesh,
        shutdown=shutdown,
        stats=stats.stats,
    ),
)

