"""Simple text input widget."""

from dataclasses import dataclass

from .base_widget import BaseWidget
from ..core import style
from ..core.event import Event, EventType
from ..render.draw_ops import draw_rect, draw_text


@dataclass
class TextBox(BaseWidget):
    text: str = ""
    focused: bool = False

    def draw(self, ctx) -> None:
        color = style.ACCENT if self.focused else style.BACKGROUND
        draw_rect(self.x, self.y, self.width, self.height, color)
        draw_text(self.text, self.x + 4, self.y + 4, style.TEXT)
        super().draw(ctx)

    def handle_event(self, ctx, event: Event) -> bool:
        if event.type == EventType.MOUSE_DOWN:
            mx, my = event.position or (0, 0)
            self.focused = (
                self.x <= mx <= self.x + self.width and self.y <= my <= self.y + self.height
            )
            return self.focused
        if self.focused and event.type == EventType.KEY_DOWN and event.key:
            if event.key == 8:  # backspace
                self.text = self.text[:-1]
            else:
                self.text += chr(event.key)
            return True
        return super().handle_event(ctx, event)
