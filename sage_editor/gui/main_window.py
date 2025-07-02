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
from .object_list import ObjectListWidget
from .property_editor import PropertyEditor
from engine.utils.log import set_stream



class EditorWindow(QMainWindow):
    """Main window hosting only a :class:`Viewport`."""

    def __init__(self, scene: Scene, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("SAGE Editor")
        self.scene = scene
        self.viewport = Viewport(scene, self)
        self.console = ConsoleWidget(self)
        self.object_list = ObjectListWidget(scene, self)
        self.prop_editor = PropertyEditor(self)

        left = QSplitter(Qt.Orientation.Vertical, self)
        left.addWidget(self.viewport)
        left.addWidget(self.console)
        left.setSizes([480, 120])

        right = QSplitter(Qt.Orientation.Vertical, self)
        right.addWidget(self.object_list)
        right.addWidget(self.prop_editor)
        right.setSizes([200, 280])

        splitter = QSplitter(self)
        splitter.setOrientation(Qt.Orientation.Horizontal)
        splitter.addWidget(left)
        splitter.addWidget(right)
        splitter.setSizes([640, 240])
        self.setCentralWidget(splitter)

        set_stream(self.console)

        self.object_list.objectSelected.connect(self.viewport.select_object)
        self.object_list.objectSelected.connect(self.prop_editor.set_object)
        self.viewport.objectSelected.connect(self.object_list.select_object)
        self.viewport.objectSelected.connect(self.prop_editor.set_object)

        self._create_menu()

    # -------------------------
    def _create_menu(self) -> None:
        bar = QMenuBar(self)
        file_menu = bar.addMenu("File")
        exit_action = QAction("Exit", self)
        exit_action.triggered.connect(QApplication.instance().quit)
        file_menu.addAction(exit_action)
        self.setMenuBar(bar)
