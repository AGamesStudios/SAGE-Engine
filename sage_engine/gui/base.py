from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from .events import Event
from .style import WidgetStyle


@dataclass
class Widget:
    x: int = 0
    y: int = 0
    width: int = 10
    height: int = 10
    style: WidgetStyle = field(default_factory=WidgetStyle)
    children: List["Widget"] = field(default_factory=list)
    parent: "Widget" | None = field(default=None, repr=False)

    on_click: Event = field(default_factory=Event, init=False)
    on_hover: Event = field(default_factory=Event, init=False)
    on_focus: Event = field(default_factory=Event, init=False)
    on_change: Event = field(default_factory=Event, init=False)
    on_keypress: Event = field(default_factory=Event, init=False)

    def add_child(self, w: "Widget") -> None:
        w.parent = self
        self.children.append(w)

    def draw(self) -> None:
        from .. import gfx
        gfx.draw_rect(self.x, self.y, self.width, self.height, self.style.bg_color)
        for child in self.children:
            child.draw()
