"""Basic Qt viewport plugin for SAGE Editor."""

from __future__ import annotations

from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout

from engine.renderers.opengl.glwidget import GLWidget


def init_editor(editor) -> None:
    """Add a simple OpenGL viewport window to the editor."""
    app = QApplication.instance()
    created = False
    if app is None:
        app = QApplication([])
        created = True
    widget = GLWidget()
    window = QWidget()
    window.setWindowTitle("SAGE Viewport")
    layout = QVBoxLayout(window)
    layout.setContentsMargins(0, 0, 0, 0)
    layout.addWidget(widget)
    window.resize(640, 480)
    window.show()
    editor.viewport = window
    if created:
        app.exec()
