"""Clickable button widget."""

from dataclasses import dataclass, field
from typing import Callable

from .base_widget import BaseWidget
from ..core import style
from ..core.event import Event, EventType
from ..render.draw_ops import draw_rect, draw_text


@dataclass
class Button(BaseWidget):
    text: str = "Button"
    on_click: Callable[[], None] | None = None
    hovered: bool = False

    def draw(self, ctx) -> None:
        color = style.HOVER if self.hovered else style.ACCENT
        draw_rect(self.x, self.y, self.width, self.height, color)
        draw_text(self.text, self.x + 4, self.y + 4, style.TEXT)
        super().draw(ctx)

    def handle_event(self, ctx, event: Event) -> bool:
        if event.type == EventType.MOUSE_MOVE:
            mx, my = event.position or (0, 0)
            self.hovered = (
                self.x <= mx <= self.x + self.width and self.y <= my <= self.y + self.height
            )
        elif event.type == EventType.MOUSE_DOWN and self.hovered:
            if self.on_click:
                self.on_click()
            return True
        return super().handle_event(ctx, event)
