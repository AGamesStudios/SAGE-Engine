"""Icon helpers used by the engine.

This module mirrors :mod:`sage_editor.icons` but contains no images.
Applications should place icon files alongside this module or bundle them
separately. The engine does not depend on the editor and falls back to
blank icons if files are missing.
"""
from __future__ import annotations

from pathlib import Path
from PyQt6.QtGui import QIcon


ICON_DIR = Path(__file__).resolve().parent / "icons"
ICON_THEME = "white"
APP_ICON_NAME = "icon.png"


def set_icon_theme(theme: str) -> None:
    """Choose ``'black'`` or ``'white'`` icon subdirectory."""
    global ICON_THEME
    if theme in {"black", "white"}:
        ICON_THEME = theme


def load_icon(name: str) -> QIcon:
    """Return a :class:`~PyQt6.QtGui.QIcon` for *name* if found."""
    path = ICON_DIR / ICON_THEME / name
    if not path.is_file():
        alt = "black" if ICON_THEME == "white" else "white"
        alt_path = ICON_DIR / alt / name
        if alt_path.is_file():
            path = alt_path
        else:
            root_path = ICON_DIR / name
            if root_path.is_file():
                path = root_path
            else:
                return QIcon()
    return QIcon(str(path))


def app_icon() -> QIcon:
    """Return the application icon."""
    return load_icon(APP_ICON_NAME)
