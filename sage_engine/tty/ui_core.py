from __future__ import annotations

from typing import List

from sage_engine.core import register

from .widgets import Widget
from .theme import load_ui_theme


class UIManager:
    def __init__(self, theme: dict | None = None) -> None:
        self.widgets: List[Widget] = []
        self.theme = theme or {}

    def add(self, widget: Widget) -> None:
        self.widgets.append(widget)

    def draw(self, tty=None) -> None:
        if tty is None:
            from sage_engine.core import get
            tty = get("tty")
        for widget in self.widgets:
            widget.draw(tty, self.theme)


ui_manager = UIManager()


def init(cfg: dict | None = None) -> None:
    theme_name = "tty_dark"
    if cfg and "theme" in cfg:
        theme_name = cfg["theme"]
    ui_manager.theme = load_ui_theme(theme_name)
    register("draw", ui_manager.draw)


__all__ = ["UIManager", "ui_manager", "init"]
