from __future__ import annotations

import logging

from PyQt6.QtWidgets import QWidget  # type: ignore[import-not-found]
from PyQt6.QtCore import QEvent

from engine.renderers.sdl_widget import register_sdlwidget

log = logging.getLogger(__name__)


class SDLWidget(QWidget):
    """Qt widget that hosts an SDL renderer."""

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self.renderer = None
        self._w = 640
        self._h = 480

    def event(self, ev: QEvent):  # pragma: no cover - gui interaction
        if ev.type() == QEvent.Type.Paint and self.renderer:
            try:
                self.renderer.present()
            except Exception:
                log.exception("SDL present failed")
        return super().event(ev)

    def width(self) -> int:  # pragma: no cover - trivial
        return self._w

    def height(self) -> int:  # pragma: no cover - trivial
        return self._h


register_sdlwidget(SDLWidget)

__all__ = ["SDLWidget"]
