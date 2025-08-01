from __future__ import annotations

from dataclasses import dataclass

from ..base import Widget


@dataclass
class Slider(Widget):
    value: float = 0.0
    min_value: float = 0.0
    max_value: float = 1.0

    def draw(self) -> None:
        super().draw()
        from ... import gfx
        pct = (self.value - self.min_value) / (self.max_value - self.min_value)
        bar_w = int(self.width * pct)
        gfx.draw_rect(self.x, self.y, bar_w, self.height, self.style.fg_color)
