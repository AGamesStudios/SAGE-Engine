from __future__ import annotations

"""Helpers for aspect ratio calculations."""

from dataclasses import dataclass


@dataclass
class Viewport:
    x: int
    y: int
    width: int
    height: int
    scale_x: float
    scale_y: float


def calculate_viewport(win_w: int, win_h: int, base_w: int, base_h: int,
                        mode: str = "fit", preserve_aspect: bool = True) -> Viewport:
    """Return the viewport rectangle to render into.

    Parameters
    ----------
    win_w, win_h: size of the window in pixels
    base_w, base_h: desired logical size
    mode: 'fit', 'fill', or 'stretch'
    preserve_aspect: whether to keep aspect ratio
    """
    if not preserve_aspect or mode == "stretch":
        scale_x = win_w / base_w
        scale_y = win_h / base_h
        return Viewport(0, 0, win_w, win_h, scale_x, scale_y)

    if mode == "fill":
        scale = max(win_w / base_w, win_h / base_h)
    else:  # fit
        scale = min(win_w / base_w, win_h / base_h)

    viewport_w = int(base_w * scale)
    viewport_h = int(base_h * scale)
    x = (win_w - viewport_w) // 2
    y = (win_h - viewport_h) // 2
    return Viewport(x, y, viewport_w, viewport_h, scale, scale)

