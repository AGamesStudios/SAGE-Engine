"""Simple depth buffer for software 3D rendering."""
from __future__ import annotations

from typing import List


class ZBuffer:
    def __init__(self, width: int, height: int) -> None:
        self.width = width
        self.height = height
        self.buffer: List[float] = [float("inf")] * (width * height)

    def clear(self) -> None:
        """Reset all depths to infinity."""
        self.buffer[:] = [float("inf")] * (self.width * self.height)

    def test_and_set(self, x: int, y: int, depth: float) -> bool:
        if not (0 <= x < self.width and 0 <= y < self.height):
            return False
        idx = y * self.width + x
        if depth < self.buffer[idx]:
            self.buffer[idx] = depth
            return True
        return False
