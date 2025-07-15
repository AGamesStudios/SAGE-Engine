"""Pluggable rendering backend loader."""

from __future__ import annotations

import os
import warnings
from importlib import import_module, metadata
from typing import Optional

from .base import RenderBackend
from .headless_backend import HeadlessBackend
from .. import sprites, ui, physics

_BACKEND: Optional[RenderBackend] = None


def _load_entry(name: str) -> Optional[RenderBackend]:
    for ep in metadata.entry_points(group="sage_render"):
        if ep.name == name:
            try:
                backend_cls = ep.load()
                return backend_cls()
            except Exception as exc:  # pragma: no cover - plugin errors
                warnings.warn(f"Failed to load render backend '{name}': {exc}")
                return None
    try:
        module = import_module(f"sage_render_{name}.backend")
        for attr in dir(module):
            if attr.endswith("Backend") and attr != "RenderBackend":
                backend_cls = getattr(module, attr)
                return backend_cls()
    except Exception:
        return None
    return None


def load_backend(name: str = "auto") -> RenderBackend:
    """Load a render backend by name."""
    if name != "auto":
        backend = _load_entry(name)
        if backend:
            print(f"Render backend: {name}")
            return backend
    else:
        for candidate in ("opengl", "headless"):
            backend = _load_entry(candidate)
            if backend:
                print(f"Render backend: {candidate}")
                return backend
    warnings.warn("No render backend available, falling back to headless")
    print("Render backend: headless")
    return HeadlessBackend()


def get_backend(name: str = "auto") -> RenderBackend:
    """Return the current render backend, loading it on first use."""
    global _BACKEND
    if name == "auto":
        name = os.environ.get("SAGE_RENDER", "auto")
    if _BACKEND is None or name != "auto":
        _BACKEND = load_backend(name)
    return _BACKEND




def draw_frame(backend: RenderBackend | None = None, *, world: physics.World | None = None) -> RenderBackend:
    """Render sprites and UI widgets in one frame."""
    if backend is None:
        backend = get_backend()
    backend.begin_frame()
    backend.draw_sprites(sprites.collect_instances())
    if world is not None and physics.debug_xray:
        backend.draw_lines(physics.collect_debug_lines(world), (1.0, 1.0, 1.0))
    backend.draw_sprites(ui.collect_instances())
    backend.end_frame()
    return backend


__all__ = ["RenderBackend", "get_backend", "load_backend", "draw_frame"]
