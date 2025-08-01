"""Gradient fill utilities for GUI widgets."""
from __future__ import annotations

from .. import gfx


def fill_rect_gradient(x: int, y: int, w: int, h: int, c1: tuple[int, int, int, int], c2: tuple[int, int, int, int], direction: str = "vertical") -> None:
    if h <= 0 or w <= 0:
        return
    if direction == "vertical":
        steps = max(h - 1, 1)
        for i in range(h):
            t = i / steps
            color = tuple(int(c1[j] * (1 - t) + c2[j] * t) for j in range(4))
            gfx.draw_rect(x, y + i, w, 1, color)
    else:
        steps = max(w - 1, 1)
        for i in range(w):
            t = i / steps
            color = tuple(int(c1[j] * (1 - t) + c2[j] * t) for j in range(4))
            gfx.draw_rect(x + i, y, 1, h, color)
