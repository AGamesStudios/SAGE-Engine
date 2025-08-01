from __future__ import annotations

from .widget import Container, Button


class GUIManager:
    """Simple GUI manager holding a root container."""

    def __init__(self) -> None:
        self.root = Container(0, 0, 0, 0)

    def draw(self) -> None:
        self.root.draw()

    def dispatch_click(self, x: int, y: int) -> None:
        self._dispatch_click(self.root, x, y)

    def _dispatch_click(self, widget: Container, x: int, y: int) -> None:
        inside = True
        if widget.width or widget.height:
            inside = widget.x <= x < widget.x + widget.width and widget.y <= y < widget.y + widget.height
        if inside:
            if isinstance(widget, Button):
                widget.click()
            for child in widget.children:
                self._dispatch_click(child, x, y)

