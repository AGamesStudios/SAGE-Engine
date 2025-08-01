from __future__ import annotations

from ..graphic import api as graphic
from .sprite import Sprite
from .sprite_batch import SpriteBatch

_commands: list[tuple[Sprite, int, int, int, int]] = []


def draw_batch(batch: SpriteBatch) -> memoryview:
    """Render all sprites from *batch* and return the framebuffer."""
    for i in range(batch.count):
        color = (
            batch.r[i],
            batch.g[i],
            batch.b[i],
            batch.a[i],
        )
        spr = batch.sprite[i]
        if spr is None:
            continue
        graphic.draw_sprite({"size": (int(batch.w[i]), int(batch.h[i])), "color": color}, int(batch.x[i]), int(batch.y[i]))
    batch.clear()
    return graphic.flush()


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
