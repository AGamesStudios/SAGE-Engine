"""Cross-platform fullscreen helpers."""

from __future__ import annotations

from typing import Any


def set_fullscreen(win: Any, enabled: bool) -> None:
    """Toggle fullscreen on the given window if supported."""
    if hasattr(win, "set_fullscreen"):
        win.set_fullscreen(enabled)
    else:
        setattr(win, "fullscreen", enabled)


def set_resolution(win: Any, width: int, height: int) -> None:
    """Change window resolution if supported."""
    if hasattr(win, "set_resolution"):
        win.set_resolution(width, height)
    else:
        win.width = width
        win.height = height

