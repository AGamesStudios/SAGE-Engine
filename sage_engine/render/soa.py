"""Sprite batch storage using Structure of Arrays."""

from __future__ import annotations
from array import array
from typing import List

MAX_SPRITES = 100_000

class SpriteBatch:
    """Preallocated container for sprite draw data."""

    def __init__(self, capacity: int = MAX_SPRITES) -> None:
        self.capacity = capacity
        self.count = 0
        self.x = array('h', [0] * capacity)
        self.y = array('h', [0] * capacity)
        self.w = array('H', [0] * capacity)
        self.h = array('H', [0] * capacity)
        self.color = array('I', [0] * capacity)

    def clear(self) -> None:
        self.count = 0

    def add(self, x: int, y: int, w: int, h: int, color: int) -> None:
        if self.count >= self.capacity:
            return
        idx = self.count
        self.x[idx] = x
        self.y[idx] = y
        self.w[idx] = w
        self.h[idx] = h
        self.color[idx] = color
        self.count += 1

