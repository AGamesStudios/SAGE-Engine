from __future__ import annotations

from dataclasses import dataclass, field
from typing import Callable, List, Any

from ..log import logger
from .math2d import make_transform


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
    transform: Transform2D = field(default_factory=Transform2D)
    logic: List[Callable[["Object", float], Any]] = field(default_factory=list)
    metadata: dict = field(default_factory=dict)

    def update(self, dt: float) -> None:
        """Call logic callbacks sequentially."""
        for func in list(self.logic):
            try:
                func(self, dt)
            except Exception:
                logger.exception("Logic error in %s", self.role)
