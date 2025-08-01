from __future__ import annotations

from .base import Widget
from .widgets.button import Button
from . import style
from ..logger import logger
from .. import gfx


class GUIManager:
    """Root container and dispatcher for GUI widgets."""

    def __init__(self) -> None:
        self.root = Widget(0, 0, 0, 0)
        self._focus: Widget | None = None
        self.debug: bool = False
        self.theme = style.DEFAULT_THEME_NAME
        style.load_theme(style.DEFAULT_THEME_NAME, style.DEFAULT_THEME)
        self._default_font = gfx.load_font(style.DEFAULT_THEME["font"], style.DEFAULT_THEME["font_size"])

    def draw(self) -> None:
        self.root.draw()
        if self.debug:
            self._draw_debug(self.root)

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

    def _draw_debug(self, widget: Widget) -> None:
        from ... import gfx
        gfx.draw_rect(widget.x, widget.y, widget.width, widget.height, (255, 0, 0, 128))
        if hasattr(widget, "text"):
            logger.debug("[gui] text=%s", getattr(widget, "text"))
        for child in widget.children:
            self._draw_debug(child)
