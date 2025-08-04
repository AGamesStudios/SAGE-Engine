from __future__ import annotations

from .model import Color


def blend(bg: Color, fg: Color) -> Color:
    """Alpha blend ``fg`` over ``bg``."""
    alpha = fg.a / 255.0
    inv = 1.0 - alpha
    r = int(fg.r * alpha + bg.r * inv)
    g = int(fg.g * alpha + bg.g * inv)
    b = int(fg.b * alpha + bg.b * inv)
    return Color(r, g, b, 255)
