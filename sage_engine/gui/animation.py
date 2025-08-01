"""GUI animation helpers using the graphic animation module."""
from __future__ import annotations

from ..graphic import animation as g_animation


def animate(widget, prop: str, start: float, end: float, duration: int, easing: str = "linear") -> None:
    g_animation.animate(widget, prop, start, end, duration, easing)
