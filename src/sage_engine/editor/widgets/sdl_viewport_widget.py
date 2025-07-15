from __future__ import annotations

from typing import TYPE_CHECKING

from sage_engine.editor.qt import SDLWidget

from ..plugins.viewport_base import _ViewportMixin

if TYPE_CHECKING:  # pragma: no cover - for type hints
    from ..plugins.editor_window import EditorWindow


class SDLViewportWidget(_ViewportMixin, SDLWidget):
    """SDL viewport widget."""

    def __init__(self, window: "EditorWindow") -> None:
        SDLWidget.__init__(self, window)
        _ViewportMixin.__init__(self, window)

    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        super().mousePressEvent(ev)

    def mouseMoveEvent(self, ev):  # pragma: no cover - gui interaction
        super().mouseMoveEvent(ev)

    def mouseReleaseEvent(self, ev):  # pragma: no cover - gui interaction
        super().mouseReleaseEvent(ev)

    def wheelEvent(self, ev):  # pragma: no cover - gui interaction
        delta = 0
        if hasattr(ev, "angleDelta"):
            delta = ev.angleDelta().y()
        elif hasattr(ev, "delta"):
            delta = ev.delta()
        if delta:
            factor = 1.1 ** (delta / 120)
            cam = self._window.camera
            cam.zoom *= factor
            if cam.zoom <= 0:
                cam.zoom = 0.1
            self._window.draw_scene(update_list=False)

