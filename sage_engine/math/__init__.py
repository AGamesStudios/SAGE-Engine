"""Basic math helpers for game logic and gizmos."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, List, Tuple
import ast
import math


@dataclass
class Vector2:
    """2D vector with simple arithmetic operations."""

    x: float
    y: float

    def __add__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2") -> "Vector2":
        return Vector2(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: float) -> "Vector2":
        return Vector2(self.x * scalar, self.y * scalar)

    def to_tuple(self) -> Tuple[float, float]:
        return self.x, self.y


@dataclass
class Vector3:
    """3D vector"""

    x: float
    y: float
    z: float

    def __add__(self, other: "Vector3") -> "Vector3":
        return Vector3(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other: "Vector3") -> "Vector3":
        return Vector3(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, scalar: float) -> "Vector3":
        return Vector3(self.x * scalar, self.y * scalar, self.z * scalar)

    def to_tuple(self) -> Tuple[float, float, float]:
        return self.x, self.y, self.z


class Matrix3:
    """Simple 3x3 matrix"""

    def __init__(self, rows: Iterable[Iterable[float]]):
        data = [list(r) for r in rows]
        if len(data) != 3 or any(len(row) != 3 for row in data):
            raise ValueError("Matrix3 requires 3x3 values")
        self.data = data

    def __matmul__(self, vec: Vector3) -> Vector3:
        x = self.data[0][0] * vec.x + self.data[0][1] * vec.y + self.data[0][2] * vec.z
        y = self.data[1][0] * vec.x + self.data[1][1] * vec.y + self.data[1][2] * vec.z
        z = self.data[2][0] * vec.x + self.data[2][1] * vec.y + self.data[2][2] * vec.z
        return Vector3(x, y, z)


_ALLOWED_NAMES = {k: getattr(math, k) for k in dir(math) if not k.startswith("_")}
_ALLOWED_NAMES.update({"pi": math.pi, "e": math.e})


def eval_expr(expr: str, **vars: float) -> float:
    """Safely evaluate a math expression."""

    tree = ast.parse(expr, mode="eval")
    allowed = (
        ast.Expression,
        ast.BinOp,
        ast.UnaryOp,
        ast.Num,
        ast.Name,
        ast.Load,
        ast.Call,
        ast.Add,
        ast.Sub,
        ast.Mult,
        ast.Div,
        ast.Pow,
        ast.Mod,
        ast.USub,
    )
    for node in ast.walk(tree):
        if not isinstance(node, allowed):
            raise ValueError("unsafe expression")
        if isinstance(node, ast.Call):
            if not isinstance(node.func, ast.Name) or node.func.id not in _ALLOWED_NAMES:
                raise ValueError("function not allowed")
    env = {**_ALLOWED_NAMES, **vars}
    return eval(compile(tree, "<expr>", "eval"), {"__builtins__": {}}, env)


def vector_lerp(v1: Vector2 | Vector3, v2: Vector2 | Vector3, t: float) -> Vector2 | Vector3:
    """Linearly interpolate between two vectors."""
    if isinstance(v1, Vector3) and isinstance(v2, Vector3):
        return Vector3(
            v1.x + (v2.x - v1.x) * t,
            v1.y + (v2.y - v1.y) * t,
            v1.z + (v2.z - v1.z) * t,
        )
    if isinstance(v1, Vector2) and isinstance(v2, Vector2):
        return Vector2(
            v1.x + (v2.x - v1.x) * t,
            v1.y + (v2.y - v1.y) * t,
        )
    raise TypeError("vector types must match")


def plot(expr: str, start: float = 0.0, end: float = 1.0, step: float = 0.1) -> List[Tuple[float, float]]:
    """Return (x, y) points for expression evaluated over a range."""
    points: List[Tuple[float, float]] = []
    x = start
    while x <= end:
        y = float(eval_expr(expr, x=x))
        points.append((x, y))
        x += step
    return points


__all__ = [
    "Vector2",
    "Vector3",
    "Matrix3",
    "eval_expr",
    "vector_lerp",
    "plot",
]
