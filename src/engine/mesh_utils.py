"""Utilities for generating simple 2D meshes."""

import math
from dataclasses import dataclass
from typing import cast

try:  # optional, used for boolean mesh operations
    from shapely.geometry import Polygon
    from shapely.geometry.base import BaseGeometry
    from shapely.ops import unary_union
except Exception:  # pragma: no cover - shapely missing
    Polygon = None  # type: ignore
    unary_union = None  # type: ignore
    from typing import Any
    BaseGeometry = Any  # type: ignore

__all__ = [
    "Mesh",
    "create_square_mesh",
    "create_triangle_mesh",
    "create_circle_mesh",
    "create_polygon_mesh",
    "union_meshes",
    "difference_meshes",
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


def union_meshes(
    meshes: list[Mesh], negatives: list[Mesh] | None = None
) -> Mesh:
    """Return a new :class:`Mesh` from ``meshes`` optionally subtracting ``negatives``."""
    if negatives and Polygon is not None and unary_union is not None:
        geom = unary_union([Polygon(m.vertices) for m in meshes])  # pyright: ignore[reportOptionalCall]
        for neg in negatives:
            geom = geom.difference(Polygon(neg.vertices))
        if isinstance(geom, Polygon):
            vertices = cast(list[tuple[float, float]], list(geom.exterior.coords)[:-1])
            inds: list[int] = []
            for i in range(1, len(vertices) - 1):
                inds.extend([0, i, i + 1])
            return Mesh(vertices, inds)
    vertices: list[tuple[float, float]] = []
    indices: list[int] = []
    offset = 0
    for mesh in meshes:
        vertices.extend(list(mesh.vertices))
        if mesh.indices is None:
            indices.extend(range(offset, offset + len(mesh.vertices)))
        else:
            indices.extend(i + offset for i in mesh.indices)
        offset += len(mesh.vertices)
    return Mesh(vertices, indices)


def difference_meshes(base: Mesh, subtract: list[Mesh]) -> Mesh:
    """Return ``base`` minus all meshes in ``subtract`` using shapely when available."""
    if Polygon is None:
        raise ImportError("shapely is required for difference_meshes")
    geom = Polygon(base.vertices)
    for neg in subtract:
        geom = geom.difference(Polygon(neg.vertices))

    polygons: list[Polygon]
    if isinstance(geom, Polygon):
        polygons = [geom]
    elif hasattr(geom, "geoms"):
        polygons = [g for g in geom.geoms if isinstance(g, Polygon)]
        if not polygons:
            raise ValueError("Difference produced unsupported geometry")
    else:
        raise ValueError("Difference produced unsupported geometry")

    vertices: list[tuple[float, float]] = []
    indices: list[int] = []
    offset = 0
    for poly in polygons:
        poly_verts = cast(list[tuple[float, float]], list(poly.exterior.coords)[:-1])
        vertices.extend(poly_verts)
        for i in range(1, len(poly_verts) - 1):
            indices.extend([offset, offset + i, offset + i + 1])
        offset += len(poly_verts)
    return Mesh(vertices, indices)
