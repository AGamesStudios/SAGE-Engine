from __future__ import annotations

from PyQt6.QtWidgets import (
    QMainWindow, QDockWidget, QWidget, QAction, QMenuBar, QApplication
)
from PyQt6.QtCore import Qt

from engine.core.scenes.scene import Scene

from .viewport import Viewport
from .plugins import load_plugins


class EditorWindow(QMainWindow):
    """Main window hosting a :class:`Viewport` and dock widgets."""

    def __init__(self, scene: Scene, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("SAGE Editor")
        self.scene = scene
        self.viewport = Viewport(scene, self)
        self.setCentralWidget(self.viewport)
        self.docks: dict[str, QDockWidget] = {}

        self._create_menu()
        load_plugins(self)

    # -------------------------
    def _create_menu(self) -> None:
        bar = QMenuBar(self)
        file_menu = bar.addMenu("File")
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(QApplication.instance().quit)
        file_menu.addAction(exit_action)
        self.setMenuBar(bar)

    def add_dock_widget(self, name: str, widget: QWidget, area: Qt.DockWidgetArea = Qt.LeftDockWidgetArea) -> QDockWidget:
        dock = QDockWidget(name, self)
        dock.setWidget(widget)
        self.addDockWidget(area, dock)
        self.docks[name] = dock
        return dock
