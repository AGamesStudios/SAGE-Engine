"""PyQt6 implementation of GuiBackend."""

from __future__ import annotations

from typing import Any, List

from PyQt6 import QtWidgets as QtW

from sage_engine.gui.base import GuiBackend, GuiEvent


class PyQtBackend(GuiBackend):
    def __init__(self) -> None:
        self._app = QtW.QApplication.instance() or QtW.QApplication([])
        self._window: QtW.QWidget | None = None

    def create_window(self, width: int, height: int, title: str) -> None:
        self._window = QtW.QWidget()
        self._window.setWindowTitle(title)
        self._window.resize(width, height)
        self._window.show()

    def process_events(self) -> List[GuiEvent]:
        self._app.processEvents()
        return []

    def present(self, texture_handle: Any) -> None:
        pass
