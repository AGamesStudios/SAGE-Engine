"""Checkbox widget."""

from dataclasses import dataclass
from typing import Callable

from .base_widget import BaseWidget
from ..core import style
from ..core.event import Event, EventType
from ..render.draw_ops import draw_rect


@dataclass
class Checkbox(BaseWidget):
    checked: bool = False
    on_change: Callable[[bool], None] | None = None

    def draw(self, ctx) -> None:
        color = style.ACCENT if self.checked else style.BACKGROUND
        draw_rect(self.x, self.y, self.width, self.height, color)
        super().draw(ctx)

    def handle_event(self, ctx, event: Event) -> bool:
        if event.type == EventType.MOUSE_DOWN:
            mx, my = event.position or (0, 0)
            if self.x <= mx <= self.x + self.width and self.y <= my <= self.y + self.height:
                self.checked = not self.checked
                if self.on_change:
                    self.on_change(self.checked)
                return True
        return super().handle_event(ctx, event)
