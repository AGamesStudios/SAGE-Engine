"""Basic Qt viewport plugin for SAGE Editor."""

from __future__ import annotations

from PyQt6.QtWidgets import (
    QApplication,
    QMainWindow,
    QWidget,
    QVBoxLayout,
    QListWidget,
    QDockWidget,
    QMenu,
)
from PyQt6.QtCore import Qt

from engine.renderers.opengl.glwidget import GLWidget


def init_editor(editor) -> None:
    """Add a main window with viewport and object list to the editor."""
    app = QApplication.instance()
    created = False
    if app is None:
        app = QApplication([])
        created = True

    gl_widget = GLWidget()
    central = QWidget()
    layout = QVBoxLayout(central)
    layout.setContentsMargins(0, 0, 0, 0)
    layout.addWidget(gl_widget)

    window = QMainWindow()
    window.setWindowTitle("SAGE Editor")
    window.setCentralWidget(central)

    objects = QListWidget()

    dock = QDockWidget("Objects", window)
    dock.setWidget(objects)
    window.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, dock)

    def context_menu(point):
        menu = QMenu(objects)
        action = menu.addAction("Create Object")

        def create_object():
            count = objects.count() + 1
            objects.addItem(f"Object {count}")

        action.triggered.connect(create_object)
        menu.exec(objects.mapToGlobal(point))

    objects.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
    objects.customContextMenuRequested.connect(context_menu)

    window.resize(800, 600)
    window.show()

    editor.viewport = gl_widget
    editor.window = window
    if created:
        app.exec()
