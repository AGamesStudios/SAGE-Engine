"""Optional Qt ``GLWidget`` for the OpenGL renderer."""

from __future__ import annotations

from importlib import import_module
import os
from typing import Type


class _StubWidget:  # pragma: no cover - fallback when Qt is absent
    """Placeholder widget if no Qt implementation is provided."""

    pass


GLWidget: Type = _StubWidget


def register_glwidget(cls: Type) -> None:
    """Register a custom Qt widget class for OpenGL rendering."""

    global GLWidget
    GLWidget = cls


path = os.getenv("SAGE_GLWIDGET")
if path:
    try:  # pragma: no cover - runtime configuration
        module, attr = path.split(":", 1)
        register_glwidget(getattr(import_module(module), attr))
    except Exception:  # pragma: no cover - invalid path
        pass


__all__ = ["GLWidget", "register_glwidget"]
