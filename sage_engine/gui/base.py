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
    on_drag_start: Event = field(default_factory=Event, init=False)
    on_drag_move: Event = field(default_factory=Event, init=False)
    on_drag_end: Event = field(default_factory=Event, init=False)
    on_drop: Event = field(default_factory=Event, init=False)

    def validate(self) -> bool:
        from ..logger import logger
        valid = True
        if self.width is None or self.height is None:
            logger.error("[gui] Invalid size on %s", type(self).__name__)
            valid = False
        elif self.width < 0 or self.height < 0:
            logger.error("[gui] Negative size on %s", type(self).__name__)
            valid = False
        if self.x is None or self.y is None:
            logger.error("[gui] Invalid position on %s", type(self).__name__)
            valid = False
        return valid

    def add_child(self, w: "Widget") -> None:
        w.parent = self
        self.children.append(w)

    def draw(self) -> None:
        if not self.validate():
            return
        from .. import gfx
        gfx.draw_rect(self.x, self.y, self.width, self.height, self.style.bg_color)
        for child in self.children:
            child.draw()
