from __future__ import annotations

from .widgets import Widget


class Editor(Widget):
    def __init__(self, x: int, y: int, text: str = "") -> None:
        self.x = x
        self.y = y
        self.text = text

    def draw(self, tty, theme) -> None:
        tty.draw_text(self.x, self.y, self.text, fg=theme.get("foreground", "white"))
