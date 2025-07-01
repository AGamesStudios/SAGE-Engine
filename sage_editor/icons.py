"""Backwards compatibility wrapper for icon helpers."""

from .gui.icons import (
    load_icon,
    set_icon_theme,
    ICON_DIR,
    ICON_THEME,
    APP_ICON_NAME,
)

__all__ = [
    "load_icon",
    "set_icon_theme",
    "ICON_DIR",
    "ICON_THEME",
    "APP_ICON_NAME",
]
