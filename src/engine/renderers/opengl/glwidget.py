"""Backwards compatibility stub for the Qt GLWidget."""
from importlib import import_module

try:  # pragma: no cover - optional dependency
    GLWidget = import_module("sage_editor.qt.glwidget").GLWidget  # type: ignore
except Exception:  # pragma: no cover - editor not installed
    class GLWidget:  # type: ignore
        """Placeholder if Qt support is unavailable."""
        pass

__all__ = ["GLWidget"]
