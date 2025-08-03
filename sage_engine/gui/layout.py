from __future__ import annotations

from dataclasses import dataclass
from typing import List

from sage_engine.gui.base import Widget


@dataclass
class LinearLayout:
    horizontal: bool = False
    spacing: int = 0
    padding: int = 0
    auto_size: bool = False

    def apply(self, container: Widget) -> None:
        pos = 0
        for child in container.children:
            if self.horizontal:
                child.x = container.x + self.padding + pos
                child.y = container.y + self.padding
                pos += child.width + self.spacing
            else:
                child.y = container.y + self.padding + pos
                child.x = container.x + self.padding
                pos += child.height + self.spacing
        if self.auto_size:
            if self.horizontal:
                container.width = pos - self.spacing + self.padding * 2
                max_h = max((c.height for c in container.children), default=0)
                container.height = max_h + self.padding * 2
            else:
                container.height = pos - self.spacing + self.padding * 2
                max_w = max((c.width for c in container.children), default=0)
                container.width = max_w + self.padding * 2


class HBoxLayout(LinearLayout):
    def __init__(self, spacing: int = 0, padding: int = 0, auto_size: bool = False):
        super().__init__(horizontal=True, spacing=spacing, padding=padding, auto_size=auto_size)


class VBoxLayout(LinearLayout):
    def __init__(self, spacing: int = 0, padding: int = 0, auto_size: bool = False):
        super().__init__(horizontal=False, spacing=spacing, padding=padding, auto_size=auto_size)


@dataclass
class GridLayout:
    cols: int
    spacing: int = 0
    padding: int = 0
    auto_size: bool = False

    def apply(self, container: Widget) -> None:
        x_off = self.padding
        y_off = self.padding
        col = 0
        max_row_height = 0
        for child in container.children:
            child.x = container.x + x_off
            child.y = container.y + y_off
            col += 1
            max_row_height = max(max_row_height, child.height)
            if col >= self.cols:
                col = 0
                x_off = self.padding
                y_off += max_row_height + self.spacing
                max_row_height = 0
            else:
                x_off += child.width + self.spacing
        if self.auto_size:
            rows = (len(container.children) + self.cols - 1) // self.cols
            total_h = self.padding * 2 + rows * max_row_height + (rows - 1) * self.spacing
            max_w = 0
            for i in range(0, len(container.children), self.cols):
                row = container.children[i:i+self.cols]
                row_w = sum(c.width for c in row) + self.spacing * (len(row)-1) + self.padding * 2
                max_w = max(max_w, row_w)
            container.width = max_w
            container.height = total_h


class AbsoluteLayout:
    def apply(self, container: Widget) -> None:
        pass


__all__ = [
    "LinearLayout",
    "HBoxLayout",
    "VBoxLayout",
    "GridLayout",
    "AbsoluteLayout",
]
