from __future__ import annotations

from typing import Sequence

_matrix: Sequence[float] = (
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
)


def set(x: float = 0.0, y: float = 0.0, zoom: float = 1.0) -> None:
    """Configure camera position and zoom."""
    global _matrix
    _matrix = (
        zoom, 0.0, -x * zoom,
        0.0, zoom, -y * zoom,
        0.0, 0.0, 1.0,
    )


def matrix() -> Sequence[float]:
    """Return the current view-projection matrix."""
    return _matrix
