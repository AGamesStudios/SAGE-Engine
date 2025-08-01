"""Simple drag and drop utilities for widgets."""
from __future__ import annotations

from .events import Event


class DragController:
    def __init__(self, widget) -> None:
        self.widget = widget
        self._dragging = False

    def start(self, x: int, y: int) -> None:
        self._dragging = True
        self.widget.on_drag_start.emit(x, y)

    def move(self, x: int, y: int) -> None:
        if self._dragging:
            self.widget.on_drag_move.emit(x, y)

    def end(self, x: int, y: int) -> None:
        if self._dragging:
            self._dragging = False
            self.widget.on_drag_end.emit(x, y)


def drop(target, source) -> None:
    target.on_drop.emit(source)
