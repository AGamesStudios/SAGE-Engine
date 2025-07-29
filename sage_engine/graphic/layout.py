"""Basic layout helpers for UI coordinates and aspect handling."""
from __future__ import annotations

from ..window.aspect import Viewport, calculate_viewport


def to_screen(x: float, y: float, viewport: Viewport) -> tuple[int, int]:
    """Convert normalized coordinates to screen space within the viewport."""
    sx = viewport.x + int(x * viewport.width)
    sy = viewport.y + int(y * viewport.height)
    return sx, sy


def safe_viewport(win_w: int, win_h: int, base_w: int, base_h: int,
                  mode: str = "fit", preserve_aspect: bool = True) -> Viewport:
    return calculate_viewport(win_w, win_h, base_w, base_h, mode, preserve_aspect)

