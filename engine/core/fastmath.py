"""Compatibility wrapper for legacy imports."""
from __future__ import annotations

from .math2d import (
    angle_to_quat,
    quat_to_angle,
    calc_rect,
    calc_matrix,
    make_transform,
    transform_point,
    make_ortho,
)

__all__ = [
    "angle_to_quat",
    "quat_to_angle",
    "calc_rect",
    "calc_matrix",
    "make_transform",
    "transform_point",
    "make_ortho",
]

