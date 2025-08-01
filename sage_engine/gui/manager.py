from __future__ import annotations

from .base import Widget
from .widgets.button import Button


class GUIManager:
    """Root container and dispatcher for GUI widgets."""

    def __init__(self) -> None:
        self.root = Widget(0, 0, 0, 0)
        self._focus: Widget | None = None
        self.theme = None

    def draw(self) -> None:
        self.root.draw()

    def dispatch_click(self, x: int, y: int) -> None:
        self._dispatch_click(self.root, x, y)

    def _dispatch_click(self, widget: Widget, x: int, y: int) -> None:
        if widget.width == 0 and widget.height == 0:
            inside = True
        else:
            inside = (
                widget.x <= x < widget.x + widget.width
                and widget.y <= y < widget.y + widget.height
            )
        if inside:
            if isinstance(widget, Button):
                widget.on_click.emit()
            for child in widget.children:
                self._dispatch_click(child, x, y)

    def set_focus(self, widget: Widget | None) -> None:
        if self._focus is widget:
            return
        if self._focus:
            self._focus.on_focus.emit(False)
        self._focus = widget
        if self._focus:
            self._focus.on_focus.emit(True)

    def get_focus(self) -> Widget | None:
        return self._focus
