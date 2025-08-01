"""High-level drawing helpers."""
from __future__ import annotations

from .. import gfx


def draw_border(x: int, y: int, w: int, h: int, color: tuple[int, int, int, int], width: int = 1) -> None:
    gfx.draw_rect(x, y, w, width, color)
    gfx.draw_rect(x, y + h - width, w, width, color)
    gfx.draw_rect(x, y, width, h, color)
    gfx.draw_rect(x + w - width, y, width, h, color)
