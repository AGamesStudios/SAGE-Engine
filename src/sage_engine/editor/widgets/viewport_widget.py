from __future__ import annotations

from typing import TYPE_CHECKING

from sage_engine.editor.qt import GLWidget

from ..plugins.viewport_base import _ViewportMixin

if TYPE_CHECKING:  # pragma: no cover - for type hints
    from ..plugins.editor_window import EditorWindow


class ViewportWidget(_ViewportMixin, GLWidget):
    """OpenGL viewport widget."""

    def __init__(self, window: "EditorWindow") -> None:
        GLWidget.__init__(self, window)
        _ViewportMixin.__init__(self, window)

