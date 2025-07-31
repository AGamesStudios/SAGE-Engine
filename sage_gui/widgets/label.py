"""Simple label widget."""

from dataclasses import dataclass

from .base_widget import BaseWidget
from ..core import style
from ..render.draw_ops import draw_text


@dataclass
class Label(BaseWidget):
    text: str = ""

    def draw(self, ctx) -> None:
        draw_text(self.text, self.x, self.y, style.TEXT)
        super().draw(ctx)
