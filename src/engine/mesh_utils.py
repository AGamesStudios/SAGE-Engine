"""Utilities for generating simple 2D meshes."""

import math
from dataclasses import dataclass

__all__ = [
    "Mesh",
    "create_square_mesh",
    "create_triangle_mesh",
    "create_circle_mesh",
    "create_polygon_mesh",
]


@dataclass
class Mesh:
    vertices: list[tuple[float, float]]
    indices: list[int] | None = None

    def translate(self, dx: float, dy: float) -> None:
        """Shift all vertices by (dx, dy)."""
        self.vertices = [(x + dx, y + dy) for x, y in self.vertices]

    def scale(self, sx: float, sy: float | None = None) -> None:
        """Scale all vertices by (sx, sy)."""
        if sy is None:
            sy = sx
        self.vertices = [(x * sx, y * sy) for x, y in self.vertices]

    def rotate(self, angle: float) -> None:
        """Rotate all vertices by ``angle`` radians."""
        cos_a = math.cos(angle)
        sin_a = math.sin(angle)
        self.vertices = [
            (x * cos_a - y * sin_a, x * sin_a + y * cos_a)
            for x, y in self.vertices
        ]


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


def create_polygon_mesh(vertices: list[tuple[float, float]]) -> Mesh:
    """Return a mesh from an arbitrary polygon defined by ``vertices``."""
    if len(vertices) < 3:
        raise ValueError("At least three vertices required")
    inds = []
    for i in range(1, len(vertices) - 1):
        inds.extend([0, i, i + 1])
    return Mesh(list(vertices), inds)
