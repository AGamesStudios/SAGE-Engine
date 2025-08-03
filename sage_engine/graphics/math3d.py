"""Minimal 3D math utilities."""
from __future__ import annotations

from dataclasses import dataclass
import math
from typing import Iterable, List


@dataclass
class Vector3:
    x: float
    y: float
    z: float

    def __add__(self, other: "Vector3") -> "Vector3":
        return Vector3(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other: "Vector3") -> "Vector3":
        return Vector3(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, k: float) -> "Vector3":
        return Vector3(self.x * k, self.y * k, self.z * k)

    def dot(self, other: "Vector3") -> float:
        return self.x * other.x + self.y * other.y + self.z * other.z

    def cross(self, other: "Vector3") -> "Vector3":
        return Vector3(
            self.y * other.z - self.z * other.y,
            self.z * other.x - self.x * other.z,
            self.x * other.y - self.y * other.x,
        )

    def to_list(self) -> List[float]:
        return [self.x, self.y, self.z]


class Matrix4:
    """4x4 matrix in row-major order."""

    def __init__(self, values: Iterable[float] | None = None) -> None:
        self.m = list(values) if values else [0.0] * 16

    @staticmethod
    def identity() -> "Matrix4":
        m = Matrix4()
        for i in range(4):
            m.m[i * 4 + i] = 1.0
        return m

    @staticmethod
    def translation(v: Vector3) -> "Matrix4":
        m = Matrix4.identity()
        m.m[12], m.m[13], m.m[14] = v.x, v.y, v.z
        return m

    @staticmethod
    def rotation_y(angle: float) -> "Matrix4":
        c = math.cos(angle)
        s = math.sin(angle)
        m = Matrix4.identity()
        m.m[0] = c
        m.m[2] = s
        m.m[8] = -s
        m.m[10] = c
        return m

    @staticmethod
    def perspective(fov: float, aspect: float, near: float, far: float) -> "Matrix4":
        f = 1.0 / math.tan(math.radians(fov) / 2.0)
        m = Matrix4()
        m.m[0] = f / aspect
        m.m[5] = f
        m.m[10] = (far + near) / (near - far)
        m.m[11] = -1.0
        m.m[14] = (2 * far * near) / (near - far)
        return m

    @staticmethod
    def look_at(eye: Vector3, target: Vector3, up: Vector3) -> "Matrix4":
        z = (eye - target)
        length = math.sqrt(z.dot(z)) or 1.0
        z = z * (1.0 / length)
        x = up.cross(z)
        length = math.sqrt(x.dot(x)) or 1.0
        x = x * (1.0 / length)
        y = z.cross(x)
        m = Matrix4.identity()
        m.m[0:4] = [x.x, y.x, z.x, 0.0]
        m.m[4:8] = [x.y, y.y, z.y, 0.0]
        m.m[8:12] = [x.z, y.z, z.z, 0.0]
        m.m[12] = -x.dot(eye)
        m.m[13] = -y.dot(eye)
        m.m[14] = -z.dot(eye)
        return m

    def __matmul__(self, other: "Matrix4") -> "Matrix4":
        r = [0.0] * 16
        for row in range(4):
            for col in range(4):
                r[row * 4 + col] = sum(
                    self.m[row * 4 + k] * other.m[k * 4 + col] for k in range(4)
                )
        return Matrix4(r)

    def transform(self, v: Vector3) -> Vector3:
        x = v.x * self.m[0] + v.y * self.m[4] + v.z * self.m[8] + self.m[12]
        y = v.x * self.m[1] + v.y * self.m[5] + v.z * self.m[9] + self.m[13]
        z = v.x * self.m[2] + v.y * self.m[6] + v.z * self.m[10] + self.m[14]
        w = v.x * self.m[3] + v.y * self.m[7] + v.z * self.m[11] + self.m[15]
        if w:
            x, y, z = x / w, y / w, z / w
        return Vector3(x, y, z)
