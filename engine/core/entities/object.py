from __future__ import annotations

from dataclasses import dataclass, field
from typing import Callable, List, Any, Optional
import itertools

from ...log import logger
from ..math2d import make_transform, transform_point

_ID_GEN = itertools.count()

def next_id() -> int:
    """Return a unique object id."""
    return next(_ID_GEN)

@dataclass
class Transform2D:
    """Simple 2D transform container."""

    x: float = 0.0
    y: float = 0.0
    scale_x: float = 1.0
    scale_y: float = 1.0
    angle: float = 0.0
    pivot_x: float = 0.0
    pivot_y: float = 0.0

    def matrix(self) -> List[float]:
        """Return a 3x3 column-major transform matrix."""
        return make_transform(
            self.x,
            self.y,
            self.scale_x,
            self.scale_y,
            self.angle,
            self.pivot_x,
            self.pivot_y,
        )


@dataclass
class Object:
    """Engine object with a role, transform and attached logic."""

    role: str
    name: Optional[str] = None
    transform: Transform2D = field(default_factory=Transform2D)
    logic: List[Callable[["Object", float], Any]] = field(default_factory=list)
    metadata: dict = field(default_factory=dict)
    id: int = field(init=False)
    parent: Optional["Object"] = field(default=None, repr=False)
    children: List["Object"] = field(default_factory=list, repr=False)

    def __post_init__(self) -> None:
        self.id = next_id()
        if self.name is None:
            self.name = self.role

    def update(self, dt: float) -> None:
        """Call logic callbacks sequentially."""
        for func in list(self.logic):
            try:
                func(self, dt)
            except Exception:
                logger.exception("Logic error in %s", self.role)

    def add_logic(self, func: Callable[["Object", float], Any]) -> None:
        """Attach a logic callback if not already present."""
        if func not in self.logic:
            self.logic.append(func)

    def remove_logic(self, func: Callable[["Object", float], Any]) -> None:
        """Remove a previously attached logic callback."""
        if func in self.logic:
            self.logic.remove(func)

    # --- transformation helpers ---
    def move(self, dx: float, dy: float) -> None:
        """Translate the object by ``dx`` and ``dy``."""
        self.transform.x += dx
        self.transform.y += dy

    def rotate(self, da: float) -> None:
        """Rotate the object by ``da`` degrees."""
        self.transform.angle += da

    def set_scale(self, sx: float, sy: Optional[float] = None) -> None:
        """Set the object's scale."""
        if sy is None:
            sy = sx
        self.transform.scale_x = sx
        self.transform.scale_y = sy

    # --- hierarchy management ---
    def add_child(self, child: "Object") -> None:
        """Attach ``child`` to this object."""
        if child.parent is self:
            return
        if child.parent is not None:
            child.parent.remove_child(child)
        child.parent = self
        self.children.append(child)

    def remove_child(self, child: "Object") -> None:
        """Detach ``child`` from this object."""
        if child in self.children:
            self.children.remove(child)
            child.parent = None

    def find_child(self, name: str) -> Optional["Object"]:
        """Recursively search for a child by name."""
        for ch in self.children:
            if ch.name == name:
                return ch
            found = ch.find_child(name)
            if found is not None:
                return found
        return None

    # --- world space helpers ---
    def world_matrix(self) -> List[float]:
        """Return the transform matrix with parents applied."""
        mat = self.transform.matrix()
        if self.parent is not None:
            pmat = self.parent.world_matrix()
            # 3x3 matrix multiply (column-major)
            mat = [
                pmat[0] * mat[0] + pmat[3] * mat[1] + pmat[6] * mat[2],
                pmat[1] * mat[0] + pmat[4] * mat[1] + pmat[7] * mat[2],
                pmat[2] * mat[0] + pmat[5] * mat[1] + pmat[8] * mat[2],
                pmat[0] * mat[3] + pmat[3] * mat[4] + pmat[6] * mat[5],
                pmat[1] * mat[3] + pmat[4] * mat[4] + pmat[7] * mat[5],
                pmat[2] * mat[3] + pmat[5] * mat[4] + pmat[8] * mat[5],
                pmat[0] * mat[6] + pmat[3] * mat[7] + pmat[6] * mat[8],
                pmat[1] * mat[6] + pmat[4] * mat[7] + pmat[7] * mat[8],
                pmat[2] * mat[6] + pmat[5] * mat[7] + pmat[8] * mat[8],
            ]
        return mat

    def world_position(self) -> tuple[float, float]:
        """Return the world position of this object."""
        if self.parent is None:
            return self.transform.x, self.transform.y
        mat = self.parent.world_matrix()
        return transform_point(mat, self.transform.x, self.transform.y)

    def destroy(self) -> None:
        """Detach from parent and clear children."""
        if self.parent is not None:
            self.parent.remove_child(self)
        for ch in list(self.children):
            ch.destroy()
