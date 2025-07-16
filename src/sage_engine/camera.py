from __future__ import annotations

from typing import Sequence, Any
import random

_x = 0.0
_y = 0.0
_zoom = 1.0
_dpi = 1.0
_aspect = 1.0
_shake = 0.0
_follow: Any | None = None


def set(
    x: float = 0.0,
    y: float = 0.0,
    zoom: float = 1.0,
    dpi: float = 1.0,
    aspect_ratio: float | None = None,
) -> None:
    """Configure camera position, zoom and DPI scale."""
    global _x, _y, _zoom, _dpi, _aspect
    _x = x
    _y = y
    _zoom = zoom
    _dpi = dpi
    if aspect_ratio is not None:
        _aspect = aspect_ratio


def pan(dx: float, dy: float) -> None:
    """Move the camera by (dx, dy) units."""
    global _x, _y
    _x += dx
    _y += dy


def zoom_to(value: float) -> None:
    """Set absolute zoom level."""
    global _zoom
    _zoom = value


def follow(obj: Any) -> None:
    """Follow the given object with ``obj.x`` and ``obj.y`` attributes."""
    global _follow
    _follow = obj


def shake(amount: float) -> None:
    """Apply a temporary shake offset of *amount* units."""
    global _shake
    _shake = max(_shake, amount)


def resize(width: int, height: int) -> None:
    """Update aspect ratio after a window resize."""
    global _aspect
    if height:
        _aspect = width / float(height)


def matrix() -> Sequence[float]:
    """Return the current view-projection matrix."""
    x, y = _x, _y
    if _follow is not None:
        x = getattr(_follow, "x", getattr(_follow, "pos", (x, y))[0])
        y = getattr(_follow, "y", getattr(_follow, "pos", (x, y))[1])
    if _shake:
        xs = random.uniform(-_shake, _shake)
        ys = random.uniform(-_shake, _shake)
    else:
        xs = ys = 0.0
    sx = _zoom / _aspect
    sy = _zoom
    if _shake:
        globals()["_shake"] *= 0.9
    return (
        sx, 0.0, -(x + xs) * sx,
        0.0, sy, -(y + ys) * sy,
        0.0, 0.0, 1.0,
    )
