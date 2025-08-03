from __future__ import annotations

from dataclasses import dataclass, field

from sage_engine.gui.base import Widget
import sage_engine.gfx as gfx


@dataclass
class ScrollView(Widget):
    scroll_x: int = 0
    scroll_y: int = 0
    content: Widget = field(default_factory=Widget)

    def add_child(self, w: Widget) -> None:
        self.content.add_child(w)

    def draw(self) -> None:
        if not self.visible:
            return
        gfx.draw_rect(self.x, self.y, self.width, self.height, self.style.bg_color)
        for child in self.content.children:
            ox, oy = child.x, child.y
            child.x = self.x + ox - self.scroll_x
            child.y = self.y + oy - self.scroll_y
            child.draw()
            child.x, child.y = ox, oy
