"""Basic 2D transform helpers."""

from __future__ import annotations

from dataclasses import dataclass
import math


@dataclass
class Transform:
    pos: tuple[float, float] = (0.0, 0.0)
    scale: tuple[float, float] = (1.0, 1.0)
    rot: float = 0.0

    def matrix(self) -> tuple[float, float, float, float, float, float]:
        sx, sy = self.scale
        c = math.cos(self.rot)
        s = math.sin(self.rot)
        a = c * sx
        b = s * sx
        c2 = -s * sy
        d = c * sy
        tx, ty = self.pos
        return (a, c2, tx, b, d, ty)
