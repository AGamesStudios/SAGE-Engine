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

    def mark_dirty(self) -> None:
        self._dirty_local = True
        self._dirty_world = True

    def set_pos(self, x: float, y: float) -> None:
        self.pos = (x, y)
        self.mark_dirty()

    def set_rot(self, rad: float) -> None:
        self.rot = rad
        self.mark_dirty()

    def set_scale(self, sx: float, sy: float) -> None:
        self.scale = (sx, sy)
        self.mark_dirty()

    def compute_local(self) -> List[float]:
        if self._dirty_local:
            self._m_local = math2d.from_transform(
                self.pos, self.rot, self.scale, self.shear, self.origin
            )
            self._dirty_local = False
        return self._m_local

    def compute_world(self, parent_world: Optional[List[float]] = None) -> List[float]:
        if self._dirty_world:
            local = self.compute_local()
            if parent_world is None:
                self._m_world = local[:]
            else:
                self._m_world = math2d.multiply(parent_world, local)
            self._dirty_world = False
        return self._m_world


@dataclass
class NodeTransform:
    transform: Transform2D = field(default_factory=Transform2D)
    parent: Optional["NodeTransform"] = None
    children: List["NodeTransform"] = field(default_factory=list)

    def add_child(self, child: "NodeTransform") -> None:
        child.parent = self
        self.children.append(child)

    def world_matrix(self) -> List[float]:
        parent_world = self.parent.world_matrix() if self.parent else None
        return self.transform.compute_world(parent_world)
