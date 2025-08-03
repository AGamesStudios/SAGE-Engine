from __future__ import annotations

from dataclasses import dataclass

from sage_engine.gui.base import Widget
import sage_engine.gfx as gfx


@dataclass
class TextInput(Widget):
    text: str = ""
    focusable: bool = True

    def draw(self) -> None:
        super().draw()
        gfx.draw_text(
            self.x + self.style.padding,
            self.y + self.style.padding,
            self.text,
            None,
            self.style.fg_color,
        )
