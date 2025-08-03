from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from sage_engine.gui.base import Widget
from sage_engine.gui.i18n import translate
import sage_engine.gfx as gfx


@dataclass
class Dropdown(Widget):
    options: List[str] = field(default_factory=list)
    selected: int = 0

    def draw(self) -> None:
        super().draw()
        if self.options:
            gfx.draw_text(
                self.x + self.style.padding,
                self.y + self.style.padding,
                translate(self.options[self.selected]),
                None,
                self.style.fg_color,
            )
