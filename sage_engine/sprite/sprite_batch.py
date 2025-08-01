from __future__ import annotations

from array import array
from typing import Optional, List

from .sprite import Sprite

DEFAULT_CAPACITY = 10000

class SpriteBatch:
    """Preallocated container for batched sprite draw calls."""

    def __init__(self, capacity: int = DEFAULT_CAPACITY) -> None:
        self.capacity = capacity
        self.count = 0
        self.x = array('f', [0.0] * capacity)
        self.y = array('f', [0.0] * capacity)
        self.w = array('f', [0.0] * capacity)
        self.h = array('f', [0.0] * capacity)
        self.r = array('B', [255] * capacity)
        self.g = array('B', [255] * capacity)
        self.b = array('B', [255] * capacity)
        self.a = array('B', [255] * capacity)
        self.sprite: List[Optional[Sprite]] = [None] * capacity

    def clear(self) -> None:
        self.count = 0

    def add(
        self,
        spr: Sprite,
        x: float,
        y: float,
        w: float,
        h: float,
        color: tuple[int, int, int, int] = (255, 255, 255, 255),
    ) -> None:
        if self.count >= self.capacity:
            return
        idx = self.count
        self.sprite[idx] = spr
        self.x[idx] = x
        self.y[idx] = y
        self.w[idx] = w
        self.h[idx] = h
        self.r[idx], self.g[idx], self.b[idx], self.a[idx] = color
        self.count += 1
