"""Lightweight transform related data structures."""
from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum
from typing import List, Optional, Tuple

from . import math2d


class Space(Enum):
    LOCAL = "local"
    PARENT = "parent"
    WORLD = "world"
    VIEW = "view"
    SCREEN = "screen"
    UI = "ui"


@dataclass
class Coord:
    x: float
    y: float
    space: Space = Space.WORLD


@dataclass
class Rect:
    x: float
    y: float
    w: float
    h: float
    space: Space = Space.WORLD


@dataclass
class Transform2D:
    """Represents a 2D transform with cached matrices."""

    pos: Tuple[float, float] = (0.0, 0.0)
    rot: float = 0.0
    scale: Tuple[float, float] = (1.0, 1.0)
    shear: Tuple[float, float] = (0.0, 0.0)
    origin: Tuple[float, float] = (0.0, 0.0)
    _m_local: List[float] = field(default_factory=math2d.identity)
    _m_world: List[float] = field(default_factory=math2d.identity)
    _dirty_local: bool = True
    _dirty_world: bool = True
    _dirty_bits: int = 0x1F  # pos|rot|scale|shear|origin

    DIRTY_POS = 1
    DIRTY_ROT = 2
    DIRTY_SCALE = 4
    DIRTY_SHEAR = 8
    DIRTY_ORIGIN = 16

    def _mark(self, bit: int) -> None:
        self._dirty_local = True
        self._dirty_world = True
        self._dirty_bits |= bit

    def set_pos(self, x: float, y: float) -> None:
        self.pos = (x, y)
        self._mark(self.DIRTY_POS)

    def set_rot(self, rad: float) -> None:
        self.rot = rad
        self._mark(self.DIRTY_ROT)

    def set_scale(self, sx: float, sy: float) -> None:
        self.scale = (sx, sy)
        self._mark(self.DIRTY_SCALE)

    def compute_local(self) -> List[float]:
        if self._dirty_local:
            self._m_local = math2d.from_transform(
                self.pos, self.rot, self.scale, self.shear, self.origin
            )
            self._dirty_local = False
            self._dirty_bits = 0
        return self._m_local

    def compute_world(self, parent_world: Optional[List[float]] = None) -> List[float]:
        if self._dirty_world:
            local = self.compute_local()
            if parent_world is None:
                self._m_world = local[:]
            else:
                self._m_world = math2d.multiply(parent_world, local)
                try:
                    from . import stats as transform_stats

                    transform_stats.stats["mul_count"] += 1
                except Exception:
                    pass
            self._dirty_world = False
        return self._m_world


@dataclass
class BaseTransform:
    """Common interface for transformable objects."""

    transform: Transform2D = field(default_factory=Transform2D)
    local_rect: Rect = field(default_factory=lambda: Rect(0.0, 0.0, 0.0, 0.0))

    def world_matrix(self, parent_world: Optional[List[float]] = None) -> List[float]:
        return self.transform.compute_world(parent_world)

    def world_aabb(self, parent_world: Optional[List[float]] = None) -> Rect:
        m = self.world_matrix(parent_world)
        return math2d.apply_to_rect(m, self.local_rect)


@dataclass
class RectangleTransform(BaseTransform):
    """Transform for standalone rectangles."""


@dataclass
class NodeTransform(BaseTransform):
    parent: Optional["NodeTransform"] = None
    children: List["NodeTransform"] = field(default_factory=list)

    def add_child(self, child: "NodeTransform") -> None:
        child.parent = self
        self.children.append(child)

    def world_matrix(self, parent_world: Optional[List[float]] = None) -> List[float]:
        if parent_world is None and self.parent:
            parent_world = self.parent.world_matrix()
        return super().world_matrix(parent_world)

    def world_aabb(self, parent_world: Optional[List[float]] = None) -> Rect:
        if parent_world is None and self.parent:
            parent_world = self.parent.world_matrix()
        return super().world_aabb(parent_world)
