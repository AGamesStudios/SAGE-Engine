"""Simple triangle mesh container."""
from __future__ import annotations

from dataclasses import dataclass
from typing import List, Tuple

from .math3d import Vector3


@dataclass
class Mesh3D:
    vertices: List[Vector3]
    triangles: List[Tuple[int, int, int]]
