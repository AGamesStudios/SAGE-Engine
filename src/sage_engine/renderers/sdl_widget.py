"""Optional Qt SDL widget registration."""

from __future__ import annotations

import os
from importlib import import_module
from typing import Type

class _StubWidget:
    """Fallback widget when Qt is unavailable."""

    pass

SDLWidget: Type = _StubWidget


def register_sdlwidget(cls: Type) -> None:
    """Register a Qt widget for SDL rendering."""
    global SDLWidget
    SDLWidget = cls


path = os.getenv("SAGE_SDLWIDGET")
if path:
    try:  # pragma: no cover - runtime configuration
        module, attr = path.split(":", 1)
        register_sdlwidget(getattr(import_module(module), attr))
    except Exception:  # pragma: no cover - invalid path
        pass

__all__ = ["SDLWidget", "register_sdlwidget"]
