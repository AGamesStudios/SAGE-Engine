from __future__ import annotations

from dataclasses import dataclass

from ..base import Widget


@dataclass
class TextInput(Widget):
    text: str = ""
    focusable: bool = True

    def draw(self) -> None:
        super().draw()
        from ... import gfx
        gfx.draw_text(
            self.x + self.style.padding,
            self.y + self.style.padding,
            self.text,
            None,
            self.style.fg_color,
        )
