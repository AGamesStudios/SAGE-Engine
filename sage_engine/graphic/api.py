"""High level graphic API using the software backend."""
from __future__ import annotations

from .backend import GraphicBackend
from .state import config
from . import fx, style

_backend = GraphicBackend()


def init(width: int, height: int) -> None:
    _backend.init(width, height)


def draw_sprite(sprite: dict, x: int, y: int) -> None:
    _backend.draw_sprite(sprite, x, y)


def draw_line(x1: int, y1: int, x2: int, y2: int, color=(255, 255, 255, 255)) -> None:
    _backend.draw_line(x1, y1, x2, y2, color)


def flush() -> memoryview:
    buf = _backend.flush()
    for eff in [config.antialiasing]:
        if eff != "None":
            fx.apply(eff, buf.obj, _backend.runtime.width, _backend.runtime.height)
    style.apply(config.style, buf.obj, _backend.runtime.width, _backend.runtime.height)
    return buf


def get_framebuffer() -> memoryview:
    return _backend.get_framebuffer()

