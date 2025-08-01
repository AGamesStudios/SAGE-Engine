from __future__ import annotations

from dataclasses import dataclass
from typing import List

from .base import Widget


@dataclass
class LinearLayout:
    horizontal: bool = False
    spacing: int = 0

    def apply(self, container: Widget) -> None:
        pos = 0
        for child in container.children:
            if self.horizontal:
                child.x = container.x + pos
                child.y = container.y
                pos += child.width + self.spacing
            else:
                child.y = container.y + pos
                child.x = container.x
                pos += child.height + self.spacing


@dataclass
class GridLayout:
    cols: int
    spacing: int = 0

    def apply(self, container: Widget) -> None:
        x_off = 0
        y_off = 0
        col = 0
        for child in container.children:
            child.x = container.x + x_off
            child.y = container.y + y_off
            col += 1
            if col >= self.cols:
                col = 0
                x_off = 0
                y_off += child.height + self.spacing
            else:
                x_off += child.width + self.spacing


class AbsoluteLayout:
    def apply(self, container: Widget) -> None:
        pass
