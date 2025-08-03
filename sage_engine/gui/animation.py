"""GUI animation helpers using the graphic animation module."""
from __future__ import annotations

from sage_engine.graphic import animation as g_animation


def animate(
    widget, prop: str, start: float, end: float, duration: int, easing: str = "linear", on_finish=None
) -> callable:
    """Animate a widget property and return a cancel callable."""
    return g_animation.animate(widget, prop, start, end, duration, easing, on_finish)
