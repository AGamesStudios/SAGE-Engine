"""Perspective camera for 3D rendering."""
from __future__ import annotations

from dataclasses import dataclass, field

from .math3d import Vector3, Matrix4


@dataclass
class Camera3D:
    """Minimal perspective camera description.

    The camera stores only intrinsic parameters.  The projection
    matrix is generated on demand for the requested aspect ratio.
    """

    position: Vector3
    look_at: Vector3
    up: Vector3 = field(default_factory=lambda: Vector3(0, 1, 0))
    fov: float = 60.0
    near: float = 0.1
    far: float = 1000.0

    def get_view_matrix(self) -> Matrix4:
        """Return the view matrix for the camera."""

        return Matrix4.look_at(self.position, self.look_at, self.up)

    def get_projection_matrix(self, aspect_ratio: float) -> Matrix4:
        """Return a perspective projection matrix for ``aspect_ratio``."""

        return Matrix4.perspective(self.fov, aspect_ratio, self.near, self.far)
