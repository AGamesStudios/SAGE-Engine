"""Utilities for generating simple 2D meshes."""
import math
from dataclasses import dataclass

__all__ = [
    "Mesh",
    "create_square_mesh",
    "create_triangle_mesh",
    "create_circle_mesh",
]


@dataclass
class Mesh:
    vertices: list[tuple[float, float]]
    indices: list[int] | None = None


def create_square_mesh(width: float = 1.0, height: float | None = None) -> Mesh:
    """Return vertices for a square or rectangle."""
    if height is None:
        height = width
    hw = width / 2
    hh = height / 2
    verts = [
        (-hw, -hh),
        (hw, -hh),
        (hw, hh),
        (-hw, hh),
    ]
    inds = [0, 1, 2, 0, 2, 3]
    return Mesh(verts, inds)


def create_triangle_mesh(size: float = 1.0) -> Mesh:
    """Return vertices for a centered equilateral triangle."""
    h = math.sqrt(3) / 2 * size
    verts = [
        (-size / 2, -h / 3),
        (size / 2, -h / 3),
        (0.0, 2 * h / 3),
    ]
    # shift so the bounding box is centred on the origin
    offset = (max(v[1] for v in verts) + min(v[1] for v in verts)) / 2
    verts = [(x, y - offset) for x, y in verts]
    return Mesh(verts, [0, 1, 2])


def create_circle_mesh(radius: float = 0.5, segments: int = 32) -> Mesh:
    """Return vertices approximating a filled circle."""
    verts = [(0.0, 0.0)]
    for i in range(segments + 1):
        ang = 2 * math.pi * i / segments
        verts.append((math.cos(ang) * radius, math.sin(ang) * radius))
    return Mesh(verts, None)
