from __future__ import annotations

from typing import Sequence

_x = 0.0
_y = 0.0
_zoom = 1.0
_dpi = 1.0


def set(x: float = 0.0, y: float = 0.0, zoom: float = 1.0, dpi: float = 1.0) -> None:
    """Configure camera position, zoom and DPI scale."""
    global _x, _y, _zoom, _dpi
    _x = x
    _y = y
    _zoom = zoom
    _dpi = dpi


def matrix() -> Sequence[float]:
    """Return the current view-projection matrix."""
    return (
        _zoom, 0.0, -_x * _zoom,
        0.0, _zoom, -_y * _zoom,
        0.0, 0.0, 1.0,
    )
