from __future__ import annotations

from ..graphic import api as graphic
from .sprite import Sprite

_commands: list[tuple[Sprite, int, int, int, int]] = []


def sprite(spr: Sprite, x: int, y: int, scale: float = 1.0, rotation: float = 0.0) -> None:
    w = int(spr.width * scale)
    h = int(spr.height * scale)
    _commands.append((spr, x, y, w, h))


def region(spr: Sprite, rect: tuple[int, int, int, int], x: int, y: int, scale: float = 1.0) -> None:
    rx, ry, rw, rh = rect
    w = int(rw * scale)
    h = int(rh * scale)
    _commands.append((spr, x, y, w, h))


def textured_rect(tex: Sprite, x: int, y: int, w: int, h: int) -> None:
    _commands.append((tex, x, y, w, h))


def flush() -> memoryview:
    for spr, x, y, w, h in _commands:
        graphic.draw_sprite({"size": (w, h)}, x, y)
    _commands.clear()
    return graphic.flush()
