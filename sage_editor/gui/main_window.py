from __future__ import annotations

from PyQt6.QtWidgets import (
    QMainWindow,
    QWidget,
    QMenuBar,
    QApplication,
    QSplitter,
    QGroupBox,
    QVBoxLayout,
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QAction

from engine.core.scenes.scene import Scene

from .viewport import Viewport
from .console import ConsoleWidget
from .object_list import ObjectListWidget
from .property_editor import PropertyEditor
from engine.utils.log import set_stream
from engine import ENGINE_VERSION



class EditorWindow(QMainWindow):
    """Main window hosting only a :class:`Viewport`."""

    def __init__(self, scene: Scene, path: str | None = None, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.setWindowTitle("SAGE Editor")
        self.scene = scene
        self.path = path
        self.viewport = Viewport(scene, self)
        self.console = ConsoleWidget(self)
        self.object_list = ObjectListWidget(scene, self)
        self.prop_editor = PropertyEditor(self)

        self.viewport_box = QGroupBox("Viewport", self)
        vbox = QVBoxLayout(self.viewport_box)
        vbox.setContentsMargins(0, 0, 0, 0)
        vbox.addWidget(self.viewport)

        self.console_box = QGroupBox("Console", self)
        cbox = QVBoxLayout(self.console_box)
        cbox.setContentsMargins(0, 0, 0, 0)
        cbox.addWidget(self.console)

        self.list_box = QGroupBox("Objects", self)
        lbox = QVBoxLayout(self.list_box)
        lbox.setContentsMargins(0, 0, 0, 0)
        lbox.addWidget(self.object_list)

        self.prop_box = QGroupBox("Properties", self)
        pbox = QVBoxLayout(self.prop_box)
        pbox.setContentsMargins(0, 0, 0, 0)
        pbox.addWidget(self.prop_editor)

        left = QSplitter(Qt.Orientation.Vertical, self)
        left.addWidget(self.viewport_box)
        left.addWidget(self.console_box)
        left.setSizes([480, 120])

        right = QSplitter(Qt.Orientation.Vertical, self)
        right.addWidget(self.list_box)
        right.addWidget(self.prop_box)
        right.setSizes([200, 280])

        splitter = QSplitter(self)
        splitter.setOrientation(Qt.Orientation.Horizontal)
        splitter.addWidget(left)
        splitter.addWidget(right)
        splitter.setSizes([640, 240])
        self.setCentralWidget(splitter)

        set_stream(self.console)
        if path:
            self.console.write(f"SAGE Engine {ENGINE_VERSION}\nLoaded: {path}\n")
        else:
            self.console.write(f"SAGE Engine {ENGINE_VERSION}\n")

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
