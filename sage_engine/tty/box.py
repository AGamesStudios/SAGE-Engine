from __future__ import annotations

from .widgets import Widget


class Box(Widget):
    def __init__(self, x: int, y: int, w: int, h: int, fg: str = "white") -> None:
        self.x, self.y, self.w, self.h = x, y, w, h
        self.fg = fg

    def draw(self, tty, theme) -> None:
        color = theme.get("accent", self.fg)
        tty.draw_rect(self.x, self.y, self.w, self.h, fg=color)
