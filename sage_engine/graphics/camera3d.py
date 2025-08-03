"""Perspective camera for 3D rendering."""
from __future__ import annotations

from dataclasses import dataclass, field

from .math3d import Vector3, Matrix4


@dataclass
class Camera3D:
    position: Vector3
    target: Vector3
    up: Vector3 = field(default_factory=lambda: Vector3(0, 1, 0))
    fov: float = 60.0
    near: float = 0.1
    far: float = 1000.0
    aspect: float = 1.0

    def view_matrix(self) -> Matrix4:
        return Matrix4.look_at(self.position, self.target, self.up)

    def projection_matrix(self) -> Matrix4:
        return Matrix4.perspective(self.fov, self.aspect, self.near, self.far)
