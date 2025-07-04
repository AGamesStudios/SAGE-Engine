from dataclasses import dataclass
from typing import Tuple


@dataclass
class Gizmo:
    """Simple debugging helper drawn in world coordinates."""

    x: float
    y: float
    size: float = 10.0
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)
    shape: str = "cross"


def cross_gizmo(x: float, y: float, *, size: float = 10.0,
                color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)) -> Gizmo:
    """Return a cross shaped gizmo."""
    return Gizmo(x, y, size, color, "cross")


def circle_gizmo(x: float, y: float, *, size: float = 10.0,
                 color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)) -> Gizmo:
    """Return a circular gizmo."""
    return Gizmo(x, y, size, color, "circle")


def square_gizmo(x: float, y: float, *, size: float = 10.0,
                 color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)) -> Gizmo:
    """Return a square gizmo."""
    return Gizmo(x, y, size, color, "square")


__all__ = ["Gizmo", "cross_gizmo", "circle_gizmo", "square_gizmo"]
