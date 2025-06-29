"""Simple yet extensible sprite painting application."""

from __future__ import annotations

import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QColorDialog,
    QToolBar, QLabel, QMessageBox
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
        color_action = QAction("Color", self)
        color_action.triggered.connect(self.choose_color)
        toolbar.addAction(color_action)

        inc_action = QAction("Width +", self)
        inc_action.triggered.connect(self.increase_width)
        toolbar.addAction(inc_action)

        dec_action = QAction("Width -", self)
        dec_action.triggered.connect(self.decrease_width)
        toolbar.addAction(dec_action)

        eraser_action = QAction("Eraser", self)
        eraser_action.setCheckable(True)
        eraser_action.toggled.connect(self.toggle_eraser)
        toolbar.addAction(eraser_action)
        self.eraser_action = eraser_action

        label = QLabel("EXPERIMENTAL", self)
        label.setStyleSheet("color: red; font-weight: bold;")
        toolbar.addWidget(label)

    def choose_color(self) -> None:
        color = QColorDialog.getColor(self.canvas.pen_color, self)
        if color.isValid():
            self.canvas.pen_color = color
            self.canvas.use_brush()
            self.eraser_action.setChecked(False)

    def increase_width(self) -> None:
        self.canvas.pen_width += 1

    def decrease_width(self) -> None:
        if self.canvas.pen_width > 1:
            self.canvas.pen_width -= 1

    def toggle_eraser(self, checked: bool) -> None:
        if checked:
            self.canvas.use_eraser()
        else:
            self.canvas.use_brush()

    def _show_disclaimer(self) -> None:
        QMessageBox.information(self, "Experimental", EXPERIMENTAL_NOTICE)


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    win = PaintWindow()
    win.show()
    return app.exec()
