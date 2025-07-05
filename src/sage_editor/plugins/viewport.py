"""Qt editor window with dockable viewport, object list and console."""

from __future__ import annotations

from PyQt6.QtWidgets import (
    QApplication,
    QMainWindow,
    QListWidget,
    QDockWidget,
    QMenu,
    QPlainTextEdit,
)
from PyQt6.QtCore import Qt

from engine.renderers.opengl.glwidget import GLWidget


class EditorWindow(QMainWindow):
    """Main editor window with dockable widgets."""

    def __init__(self) -> None:
        super().__init__()
        self.setWindowTitle("SAGE Editor")

        self.viewport = GLWidget(self)
        self.console = QPlainTextEdit(self)

        self.setCentralWidget(self.viewport)

        self.objects = QListWidget()
        obj_dock = QDockWidget("Objects", self)
        obj_dock.setObjectName("ObjectsDock")
        obj_dock.setWidget(self.objects)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)

        con_dock = QDockWidget("Console", self)
        con_dock.setObjectName("ConsoleDock")
        con_dock.setWidget(self.console)
        self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, con_dock)

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

    window = EditorWindow()
    window.resize(800, 600)
    window.show()

    editor.window = window
    editor.viewport = window.viewport
    if created:
        app.exec()
