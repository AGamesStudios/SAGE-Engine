"""Utilities for generating simple 2D meshes."""

from __future__ import annotations

import math
from dataclasses import dataclass
from typing import Any, TYPE_CHECKING, cast

try:  # optional, used for boolean mesh operations
    from shapely.geometry import Polygon as _Polygon
    from shapely.geometry.base import BaseGeometry as _BaseGeometry
    from shapely.ops import triangulate, unary_union
except Exception:  # pragma: no cover - shapely missing
    _Polygon = None  # type: ignore
    unary_union = None  # type: ignore
    _BaseGeometry = Any  # type: ignore

BaseGeometry = _BaseGeometry

if TYPE_CHECKING:  # keep type hints when shapely is missing
    from shapely.geometry import Polygon
else:  # pragma: no cover - runtime fallback
    Polygon = Any  # type: ignore

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

    def apply_matrix(self, mat: list[float]) -> None:
        """Transform vertices by a 3x3 matrix."""
        from .core.math2d import transform_point

        self.vertices = [transform_point(mat, x, y) for x, y in self.vertices]


def _geom_to_mesh(geom: BaseGeometry) -> "Mesh":
    """Return a triangulated :class:`Mesh` from ``geom``."""
    if _Polygon is None:
        raise ImportError("shapely is required for triangulation")
    polygons: list[Polygon]
    if isinstance(geom, _Polygon):
        polygons = [geom]
    elif hasattr(geom, "geoms"):
        polygons = [g for g in cast(Any, geom).geoms if isinstance(g, _Polygon)]
    else:
        raise ValueError("Unsupported geometry")

    vertices: list[tuple[float, float]] = []
    indices: list[int] = []
    offset = 0
    for poly in polygons:
        for tri in triangulate(poly):
            pts = cast(list[tuple[float, float]], list(tri.exterior.coords)[:-1])
            if len(pts) != 3:
                continue
            vertices.extend(pts)
            indices.extend([offset, offset + 1, offset + 2])
            offset += 3

    return Mesh(vertices, indices)

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
    if negatives:
        if _Polygon is None or unary_union is None:
            raise ImportError("shapely is required for union_meshes")
        geom = unary_union([_Polygon(m.vertices) for m in meshes])  # pyright: ignore[reportOptionalCall]
        for neg in negatives:
            geom = geom.difference(_Polygon(neg.vertices))
        return _geom_to_mesh(geom)
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
    if _Polygon is None:
        raise ImportError("shapely is required for difference_meshes")
    geom = _Polygon(base.vertices)
    for neg in subtract:
        geom = geom.difference(_Polygon(neg.vertices))

    return _geom_to_mesh(geom)
