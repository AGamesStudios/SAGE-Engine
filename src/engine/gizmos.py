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
    frames: int | None = 1
    filled: bool = False


def cross_gizmo(
    x: float,
    y: float,
    *,
    size: float = 10.0,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
    frames: int | None = 1,
    filled: bool = False,
) -> Gizmo:
    """Return a cross shaped gizmo."""
    return Gizmo(x, y, size, color, thickness, "cross", None, frames, filled)


def circle_gizmo(
    x: float,
    y: float,
    *,
    size: float = 10.0,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
    frames: int | None = 1,
    filled: bool = False,
) -> Gizmo:
    """Return a circular gizmo."""
    return Gizmo(x, y, size, color, thickness, "circle", None, frames, filled)


def square_gizmo(
    x: float,
    y: float,
    *,
    size: float = 10.0,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
    frames: int | None = 1,
    filled: bool = False,
) -> Gizmo:
    """Return a square gizmo."""
    return Gizmo(x, y, size, color, thickness, "square", None, frames, filled)


def polyline_gizmo(
    points: Iterable[Tuple[float, float]],
    *,
    color: Tuple[float, float, float, float] = (1.0, 1.0, 1.0, 1.0),
    thickness: float = 2.0,
    frames: int | None = 1,
) -> Gizmo:
    """Return a polyline gizmo defined by ``points`` in world units."""
    g = Gizmo(0.0, 0.0, 0.0, color, thickness, "polyline", points, frames)
    return g


__all__ = [
    "Gizmo",
    "cross_gizmo",
    "circle_gizmo",
    "square_gizmo",
    "polyline_gizmo",
]
