from __future__ import annotations

from PyQt6.QtWidgets import (
    QMainWindow,
    QWidget,
    QMenuBar,
    QApplication,
    QVBoxLayout,
)
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

        container = QWidget(self)
        layout = QVBoxLayout(container)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        layout.addWidget(self.viewport)
        layout.addWidget(self.console)
        self.setCentralWidget(container)

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
