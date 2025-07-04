from dataclasses import dataclass
from typing import Iterable, Tuple


@dataclass
class Gizmo:
    """Simple debugging helper drawn in world coordinates."""

    x: float
    y: float
    size: float = 10.0
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0)
    thickness: float = 2.0
    shape: str = "cross"
    vertices: Iterable[Tuple[float, float]] | None = None


def cross_gizmo(
    x: float,
    y: float,
    *,
    size: float = 10.0,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
) -> Gizmo:
    """Return a cross shaped gizmo."""
    return Gizmo(x, y, size, color, thickness, "cross")


def circle_gizmo(
    x: float,
    y: float,
    *,
    size: float = 10.0,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
) -> Gizmo:
    """Return a circular gizmo."""
    return Gizmo(x, y, size, color, thickness, "circle")


def square_gizmo(
    x: float,
    y: float,
    *,
    size: float = 10.0,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
) -> Gizmo:
    """Return a square gizmo."""
    return Gizmo(x, y, size, color, thickness, "square")


def polyline_gizmo(
    points: Iterable[Tuple[float, float]],
    *,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
) -> Gizmo:
    """Return a polyline gizmo defined by ``points`` in world units."""
    g = Gizmo(0.0, 0.0, 0.0, color, thickness, "polyline", points)
    return g


__all__ = [
    "Gizmo",
    "cross_gizmo",
    "circle_gizmo",
    "square_gizmo",
    "polyline_gizmo",
]
