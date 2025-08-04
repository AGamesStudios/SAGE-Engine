from __future__ import annotations

from .widgets import Widget


class List(Widget):
    def __init__(self, x: int, y: int, items: list[str], fg: str = "white") -> None:
        self.x, self.y = x, y
        self.items = items
        self.fg = fg

    def draw(self, tty, theme) -> None:
        color = theme.get("foreground", self.fg)
        for i, text in enumerate(self.items):
            tty.draw_text(self.x, self.y + i, text, fg=color)
