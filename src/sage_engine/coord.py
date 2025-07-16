"""Helpers for converting between screen pixels and world units."""

from __future__ import annotations

from dataclasses import dataclass
from . import camera


@dataclass
class CoordSystem:
    """Convert between world units and screen pixels."""

    dpi: float = 1.0

    def world_to_screen(self, x: float, y: float) -> tuple[float, float]:
        cx, cy, zoom = camera._x, camera._y, camera._zoom
        return ((x - cx) * zoom * self.dpi, (y - cy) * zoom * self.dpi)

    def screen_to_world(self, px: float, py: float) -> tuple[float, float]:
        cx, cy, zoom = camera._x, camera._y, camera._zoom
        return (px / (zoom * self.dpi) + cx, py / (zoom * self.dpi) + cy)


DEFAULT = CoordSystem()


@dataclass
class WorldSpace:
    x: float
    y: float

    def to_screen(self, cs: CoordSystem = DEFAULT) -> tuple[float, float]:
        return cs.world_to_screen(self.x, self.y)

    @classmethod
    def from_screen(cls, px: float, py: float, cs: CoordSystem = DEFAULT) -> "WorldSpace":
        x, y = cs.screen_to_world(px, py)
        return cls(x, y)


@dataclass
class ScreenSpace:
    x: float
    y: float

    def to_world(self, cs: CoordSystem = DEFAULT) -> tuple[float, float]:
        return cs.screen_to_world(self.x, self.y)


@dataclass
class UIRect:
    x: float
    y: float
    w: float
    h: float

    def to_screen(self) -> tuple[float, float, float, float]:
        return (self.x, self.y, self.w, self.h)


def world_to_screen(x: float, y: float) -> tuple[float, float]:
    return DEFAULT.world_to_screen(x, y)


def screen_to_world(px: float, py: float) -> tuple[float, float]:
    return DEFAULT.screen_to_world(px, py)
