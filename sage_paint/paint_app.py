"""Simple yet extensible sprite painting application."""

from __future__ import annotations

import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QColorDialog,
    QToolBar, QLabel, QMessageBox, QActionGroup
)
from PyQt6.QtGui import QAction
from PyQt6.QtCore import Qt

from .canvas import Canvas

EXPERIMENTAL_NOTICE = (
    "SAGE Paint is experimental. Features may change and stability is not guaranteed."
)


class PaintWindow(QMainWindow):
    """Main window hosting a :class:`Canvas` and basic drawing tools."""

    def __init__(self):
        super().__init__()
        self.setWindowTitle("SAGE Paint (Experimental)")
        self.canvas = Canvas()
        self.setCentralWidget(self.canvas)
        self._init_toolbar()
        self._show_disclaimer()

    # Toolbar ------------------------------------------------------------

    def _init_toolbar(self) -> None:
        toolbar = QToolBar(self)
        self.addToolBar(toolbar)
        group = QActionGroup(self)
        brush_action = QAction("Brush", self)
        brush_action.setCheckable(True)
        brush_action.setChecked(True)
        brush_action.triggered.connect(lambda: self.canvas.use_brush())
        group.addAction(brush_action)
        toolbar.addAction(brush_action)

        eraser_action = QAction("Eraser", self)
        eraser_action.setCheckable(True)
        eraser_action.triggered.connect(lambda: self.canvas.use_eraser())
        group.addAction(eraser_action)
        toolbar.addAction(eraser_action)
        self.brush_action = brush_action
        self.eraser_action = eraser_action

        color_action = QAction("Color", self)
        color_action.triggered.connect(self.choose_color)
        toolbar.addAction(color_action)

        inc_action = QAction("Width +", self)
        inc_action.triggered.connect(self.increase_width)
        toolbar.addAction(inc_action)

        dec_action = QAction("Width -", self)
        dec_action.triggered.connect(self.decrease_width)
        toolbar.addAction(dec_action)

        toolbar.addSeparator()
        zoom_in = QAction("Zoom +", self)
        zoom_in.triggered.connect(lambda: self.canvas.zoom(1.2))
        toolbar.addAction(zoom_in)

        zoom_out = QAction("Zoom -", self)
        zoom_out.triggered.connect(lambda: self.canvas.zoom(1/1.2))
        toolbar.addAction(zoom_out)

        label = QLabel("EXPERIMENTAL", self)
        label.setStyleSheet("color: red; font-weight: bold;")
        toolbar.addWidget(label)

    def choose_color(self) -> None:
        color = QColorDialog.getColor(self.canvas.pen_color, self)
        if color.isValid():
            self.canvas.pen_color = color
            self.brush_action.setChecked(True)

    def increase_width(self) -> None:
        self.canvas.pen_width += 1

    def decrease_width(self) -> None:
        if self.canvas.pen_width > 1:
            self.canvas.pen_width -= 1

    def _show_disclaimer(self) -> None:
        QMessageBox.information(self, "Experimental", EXPERIMENTAL_NOTICE)


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    win = PaintWindow()
    win.show()
    return app.exec()
