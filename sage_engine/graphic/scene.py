from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Tuple, Any

from .color import to_rgba


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
    items: List[Any] = field(default_factory=list)

    def add(self, rect: Rect) -> None:
        self.items.append(rect)


@dataclass
class Group:
    items: List[Any] = field(default_factory=list)
    state: tuple | None = None
    timestamp: int = 0

    def add(self, item: Any) -> None:
        self.items.append(item)


class Scene:
    def __init__(self) -> None:
        self.layers: List[Layer] = []
        self._tick = 0

    def add(self, layer: Layer) -> None:
        self.layers.append(layer)

    def rect(self, container: Layer | Group, x: int, y: int, w: int, h: int, color) -> None:
        self._tick += 1
        container.add(Rect(x, y, w, h, to_rgba(color), self._tick))

    def group(self, container: Layer | Group, state: tuple | None = None) -> Group:
        self._tick += 1
        g = Group(state=state, timestamp=self._tick)
        container.add(g)
        return g

    def render(self) -> None:
        from .. import gfx
        for layer in sorted(self.layers, key=lambda l: l.z):
            gfx.state.z = layer.z
            for item in sorted(layer.items, key=lambda it: getattr(it, "timestamp", 0)):
                self._render_item(item)
        gfx.state.z = 0

    def _render_item(self, item: Any) -> None:
        from .. import gfx
        if isinstance(item, Rect):
            gfx.draw_rect(item.x, item.y, item.w, item.h, item.color)
        elif isinstance(item, Group):
            if item.state is not None:
                gfx.push_state()
                gfx.state.restore(item.state)
            for sub in sorted(item.items, key=lambda it: getattr(it, "timestamp", 0)):
                self._render_item(sub)
            if item.state is not None:
                gfx.pop_state()
