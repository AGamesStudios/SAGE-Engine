from __future__ import annotations

from PyQt6.QtWidgets import (
    QMainWindow,
    QWidget,
    QMenuBar,
    QApplication,
    QSplitter,
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QAction

from engine.core.scenes.scene import Scene

from .viewport import Viewport
from .console import ConsoleWidget
from engine.utils.log import set_stream



class EditorWindow(QMainWindow):
    """Main window hosting only a :class:`Viewport`."""

    def __init__(self, scene: Scene, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("SAGE Editor")
        self.scene = scene
        self.viewport = Viewport(scene, self)
        self.console = ConsoleWidget(self)

        splitter = QSplitter(self)
        splitter.setOrientation(Qt.Orientation.Vertical)
        splitter.addWidget(self.viewport)
        splitter.addWidget(self.console)
        splitter.setSizes([480, 120])
        self.setCentralWidget(splitter)

        set_stream(self.console)

        self._create_menu()

    # -------------------------
    def _create_menu(self) -> None:
        bar = QMenuBar(self)
        file_menu = bar.addMenu("File")
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(QApplication.instance().quit)
        file_menu.addAction(exit_action)
        self.setMenuBar(bar)
