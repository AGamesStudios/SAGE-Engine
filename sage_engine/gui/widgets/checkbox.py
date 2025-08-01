from __future__ import annotations

from dataclasses import dataclass

from ..base import Widget


@dataclass
class Checkbox(Widget):
    checked: bool = False

    def draw(self) -> None:
        super().draw()
        if self.checked:
            from ... import gfx
            gfx.draw_line(
                self.x,
                self.y,
                self.x + self.width,
                self.y + self.height,
                1,
                self.style.fg_color,
            )
