"""Simple panel container."""

from dataclasses import dataclass

from .base_widget import BaseWidget
from ..core import style
from ..render.draw_ops import draw_rect


@dataclass
class Panel(BaseWidget):
    def draw(self, ctx) -> None:
        draw_rect(self.x, self.y, self.width, self.height, style.BACKGROUND)
        super().draw(ctx)
