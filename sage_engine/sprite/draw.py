from __future__ import annotations

from .. import gfx
from .sprite import Sprite
from .sprite_batch import SpriteBatch
from .atlas import get as get_from_atlas

_commands: list[tuple[Sprite, int, int, int, int]] = []


def draw_batch(batch: SpriteBatch) -> memoryview:
    """Render all sprites from *batch* and return the framebuffer."""
    for i in range(batch.count):
        spr = batch.sprite[i]
        if spr is None:
            continue
        spr.frame_rect = (spr.frame_rect[0], spr.frame_rect[1], int(batch.w[i]), int(batch.h[i]))
        gfx.draw_sprite(spr, int(batch.x[i]), int(batch.y[i]))
    batch.clear()
    buf = gfx.end_frame()
    gfx.flush_frame()
    return buf


def sprite(spr: Sprite, x: int, y: int, scale: float = 1.0, rotation: float = 0.0) -> None:
    w = int(spr.frame_rect[2] * scale)
    h = int(spr.frame_rect[3] * scale)
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
        spr.frame_rect = (spr.frame_rect[0], spr.frame_rect[1], w, h)
        gfx.draw_sprite(spr, x, y)
    _commands.clear()
    buf = gfx.end_frame()
    gfx.flush_frame()
    return buf


def sprite_from_atlas(name: str, x: int, y: int, scale: float = 1.0) -> None:
    spr = get_from_atlas(name)
    if spr is not None:
        sprite(spr, x, y, scale)
