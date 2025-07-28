from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Tuple

from .color import to_rgba
from .. import gfx


@dataclass
class Rect:
    x: int
    y: int
    w: int
    h: int
    color: Tuple[int, int, int, int]
    timestamp: int = 0


@dataclass
class Layer:
    z: int = 0
    items: List[Rect] = field(default_factory=list)

    def add(self, rect: Rect) -> None:
        self.items.append(rect)


class Scene:
    def __init__(self) -> None:
        self.layers: List[Layer] = []
        self._tick = 0

    def add(self, layer: Layer) -> None:
        self.layers.append(layer)

    def rect(self, layer: Layer, x: int, y: int, w: int, h: int, color) -> None:
        self._tick += 1
        layer.add(Rect(x, y, w, h, to_rgba(color), self._tick))

    def render(self) -> None:
        for layer in sorted(self.layers, key=lambda l: l.z):
            gfx.state.z = layer.z
            for item in sorted(layer.items, key=lambda it: it.timestamp):
                gfx.draw_rect(item.x, item.y, item.w, item.h, item.color)
        gfx.state.z = 0
