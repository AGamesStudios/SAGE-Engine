"""Minimal 2D matrix helpers used by the transform module."""
from __future__ import annotations

import math
from typing import Iterable, List, Tuple

Matrix = List[float]  # length 9, row major


def identity() -> Matrix:
    return [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]


def multiply(a: Matrix, b: Matrix) -> Matrix:
    return [
        a[0] * b[0] + a[1] * b[3] + a[2] * b[6],
        a[0] * b[1] + a[1] * b[4] + a[2] * b[7],
        a[0] * b[2] + a[1] * b[5] + a[2] * b[8],
        a[3] * b[0] + a[4] * b[3] + a[5] * b[6],
        a[3] * b[1] + a[4] * b[4] + a[5] * b[7],
        a[3] * b[2] + a[4] * b[5] + a[5] * b[8],
        a[6] * b[0] + a[7] * b[3] + a[8] * b[6],
        a[6] * b[1] + a[7] * b[4] + a[8] * b[7],
        a[6] * b[2] + a[7] * b[5] + a[8] * b[8],
    ]


def apply_to_point(m: Matrix, x: float, y: float) -> Tuple[float, float]:
    return (
        m[0] * x + m[1] * y + m[2],
        m[3] * x + m[4] * y + m[5],
    )


def from_transform(
    pos: Tuple[float, float],
    rot: float,
    scale: Tuple[float, float],
    shear: Tuple[float, float],
    origin: Tuple[float, float],
) -> Matrix:
    """Create a matrix from transform parameters."""
    sx, sy = scale
    shx, shy = shear
    ox, oy = origin
    c = math.cos(rot)
    s = math.sin(rot)
    m0 = sx * c + shy * s
    m1 = -sx * s + shy * c
    m3 = shx * c + sy * s
    m4 = -shx * s + sy * c
    tx = pos[0] - m0 * ox - m1 * oy
    ty = pos[1] - m3 * ox - m4 * oy
    return [m0, m1, tx, m3, m4, ty, 0.0, 0.0, 1.0]
