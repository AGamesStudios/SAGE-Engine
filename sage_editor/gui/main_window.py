from __future__ import annotations

from PyQt6.QtWidgets import (
    QMainWindow,
    QWidget,
    QMenuBar,
    QApplication,
    QSplitter,
    QGroupBox,
    QVBoxLayout,
    QScrollArea,
    QTabWidget,
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QAction

from engine.core.scenes.scene import Scene

from .viewport import Viewport
from .console import ConsoleWidget
from .object_list import ObjectTreeWidget
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
        self.object_list = ObjectTreeWidget(scene, self)
        self.prop_editor = PropertyEditor(self)

        self.viewport_tabs = QTabWidget(self)
        self.viewport_tabs.setTabsClosable(True)
        self.viewport_tabs.addTab(self.viewport, "Viewport")
        self.viewport_tabs.tabCloseRequested.connect(
            lambda idx: self.viewport_tabs.removeTab(idx)
        )

        self.console_box = QGroupBox("Console", self)
        cbox = QVBoxLayout(self.console_box)
        cbox.setContentsMargins(0, 0, 0, 0)
        self.console_scroll = QScrollArea(self.console_box)
        self.console_scroll.setWidgetResizable(True)
        self.console_scroll.setWidget(self.console)
        cbox.addWidget(self.console_scroll)

        self.list_box = QGroupBox("Objects", self)
        lbox = QVBoxLayout(self.list_box)
        lbox.setContentsMargins(0, 0, 0, 0)
        self.list_scroll = QScrollArea(self.list_box)
        self.list_scroll.setWidgetResizable(True)
        self.list_scroll.setWidget(self.object_list)
        lbox.addWidget(self.list_scroll)

        self.prop_box = QGroupBox("Properties", self)
        pbox = QVBoxLayout(self.prop_box)
        pbox.setContentsMargins(0, 0, 0, 0)
        self.prop_scroll = QScrollArea(self.prop_box)
        self.prop_scroll.setWidgetResizable(True)
        self.prop_scroll.setWidget(self.prop_editor)
        pbox.addWidget(self.prop_scroll)

        left = QSplitter(Qt.Orientation.Vertical, self)
        left.addWidget(self.viewport_tabs)
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
