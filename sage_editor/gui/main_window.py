from __future__ import annotations

from PyQt6.QtWidgets import QMainWindow, QWidget, QMenuBar, QApplication
from PyQt6.QtGui import QAction

from engine.core.scenes.scene import Scene

from .viewport import Viewport



class EditorWindow(QMainWindow):
    """Main window hosting only a :class:`Viewport`."""

    def __init__(self, scene: Scene, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("SAGE Editor")
        self.scene = scene
        self.viewport = Viewport(scene, self)
        self.setCentralWidget(self.viewport)

        self._create_menu()

    # -------------------------
    def _create_menu(self) -> None:
        bar = QMenuBar(self)
        file_menu = bar.addMenu("File")
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(QApplication.instance().quit)
        file_menu.addAction(exit_action)
        self.setMenuBar(bar)
