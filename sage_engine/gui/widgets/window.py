from __future__ import annotations

from dataclasses import dataclass, field

from ..base import Widget
from ..events import Event


@dataclass
class Window(Widget):
    """Simple movable and closable window."""
    title: str = ""
    movable: bool = True
    closable: bool = True
    header_height: int = 20
    on_close: Event = field(default_factory=Event, init=False)
    _drag_offset: tuple[int, int] | None = field(default=None, init=False, repr=False)

    def start_drag(self, x: int, y: int) -> None:
        if not self.movable:
            return
        if self.contains_point(x, y) and y - self.y <= self.header_height:
            self._drag_offset = (x - self.x, y - self.y)

    def drag(self, x: int, y: int) -> None:
        if self._drag_offset:
            ox, oy = self._drag_offset
            self.x = x - ox
            self.y = y - oy

    def end_drag(self, _: int, __: int) -> None:
        self._drag_offset = None

    def close(self) -> None:
        self.visible = False
        self.on_close.emit()

    def draw(self) -> None:
        if not self.visible:
            return
        super().draw()
        from ... import gfx
        gfx.draw_rect(self.x, self.y, self.width, self.header_height, self.style.fg_color)
        gfx.draw_text(self.x + 4, self.y + 4, self.title, None, self.style.bg_color)
