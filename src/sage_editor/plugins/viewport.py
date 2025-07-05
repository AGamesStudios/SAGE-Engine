"""Qt editor window with dockable viewport, object list and console."""

from __future__ import annotations

from PyQt6.QtWidgets import (
    QApplication,
    QMainWindow,
    QListWidget,
    QDockWidget,
    QMenu,
    QPlainTextEdit,
    QMenuBar,
    QToolBar,
    QSplitter,
)
from PyQt6.QtGui import QAction
from PyQt6.QtCore import Qt

from engine.renderers.opengl.glwidget import GLWidget


class EditorWindow(QMainWindow):
    """Main editor window with dockable widgets."""

    def __init__(self, menus=None, toolbar=None) -> None:
        super().__init__()
        self.setWindowTitle("SAGE Editor")

        self.viewport = GLWidget(self)
        self.console = QPlainTextEdit(self)
        self.properties = QPlainTextEdit(self)
        self.resources = QListWidget()

        self.splitter = QSplitter(Qt.Orientation.Vertical, self)
        self.splitter.addWidget(self.viewport)
        self.splitter.addWidget(self.console)
        self.setCentralWidget(self.splitter)

        menubar = QMenuBar(self)
        self.setMenuBar(menubar)
        if menus:
            for title, cb in menus:
                action = QAction(title, self)
                action.triggered.connect(cb)
                menubar.addAction(action)

        tbar = QToolBar(self)
        self.addToolBar(tbar)
        if toolbar:
            for title, cb in toolbar:
                action = QAction(title, self)
                action.triggered.connect(cb)
                tbar.addAction(action)

        self.objects = QListWidget()
        obj_dock = QDockWidget("Objects", self)
        obj_dock.setObjectName("ObjectsDock")
        obj_dock.setWidget(self.objects)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)

        res_dock = QDockWidget("Resources", self)
        res_dock.setObjectName("ResourcesDock")
        res_dock.setWidget(self.resources)
        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, res_dock)

        prop_dock = QDockWidget("Properties", self)
        prop_dock.setObjectName("PropertiesDock")
        prop_dock.setWidget(self.properties)
        self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, prop_dock)

        self.objects.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.objects.customContextMenuRequested.connect(self._context_menu)

    def _context_menu(self, point):
        menu = QMenu(self.objects)
        action = menu.addAction("Create Object")

        def create_object() -> None:
            count = self.objects.count() + 1
            self.objects.addItem(f"Object {count}")

        action.triggered.connect(create_object)
        menu.exec(self.objects.mapToGlobal(point))


def init_editor(editor) -> None:
    """Launch the main editor window and attach it to *editor*."""
    app = QApplication.instance()
    created = False
    if app is None:
        app = QApplication([])
        created = True

    window = EditorWindow(editor._menus, editor._toolbar)
    window.resize(800, 600)
    window.show()

    editor.window = window
    editor.viewport = window.viewport
    if created:
        app.exec()
