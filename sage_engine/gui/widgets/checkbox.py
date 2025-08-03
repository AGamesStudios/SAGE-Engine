from __future__ import annotations

from dataclasses import dataclass

from sage_engine.gui.base import Widget
import sage_engine.gfx as gfx


@dataclass
class Checkbox(Widget):
    checked: bool = False

    def draw(self) -> None:
        super().draw()
        if self.checked:
            gfx.draw_line(
                self.x,
                self.y,
                self.x + self.width,
                self.y + self.height,
                1,
                self.style.fg_color,
            )
