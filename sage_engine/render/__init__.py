"""Render system that exposes a software backend."""

from __future__ import annotations

from sage_engine.core import register, expose, get
from .backends.software import SoftwareRenderer

_renderer: SoftwareRenderer | None = None


def boot(_cfg: dict | None = None) -> None:
    window = get("window")
    global _renderer
    _renderer = SoftwareRenderer(window.width, window.height)
    expose("render", _renderer)


def draw() -> None:
    if _renderer:
        _renderer.begin_frame()


def flush() -> None:
    if _renderer:
        _renderer.end_frame()


def shutdown() -> None:
    pass


register("boot", boot)
register("draw", draw)
register("flush", flush)
register("shutdown", shutdown)
