"""Simple sprite painting tool."""

from __future__ import annotations

import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QColorDialog,
    QToolBar, QAction, QLabel, QMessageBox,
)
from PyQt6.QtGui import QPainter, QImage, QPen, QColor
from PyQt6.QtCore import Qt, QPoint

EXPERIMENTAL_NOTICE = (
    "SAGE Paint is experimental. Features may change and stability is not guaranteed."
)


class Canvas(QWidget):
    def __init__(self, width: int = 256, height: int = 256, parent: QWidget | None = None):
        super().__init__(parent)
        self.image = QImage(width, height, QImage.Format.Format_ARGB32)
        self.image.fill(QColor(0, 0, 0, 0))
        self._drawing = False
        self.pen_color = QColor("black")
        self.pen_width = 1
        self._last_pos = QPoint()

    def mousePressEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self._drawing = True
            self._last_pos = event.position().toPoint()

    def mouseMoveEvent(self, event):
        if self._drawing:
            painter = QPainter(self.image)
            pen = QPen(self.pen_color, self.pen_width, Qt.PenStyle.SolidLine,
                       Qt.PenCapStyle.RoundCap, Qt.PenJoinStyle.RoundJoin)
            painter.setPen(pen)
            painter.drawLine(self._last_pos, event.position().toPoint())
            painter.end()
            self._last_pos = event.position().toPoint()
            self.update()

    def mouseReleaseEvent(self, event):
        if event.button() == Qt.MouseButton.LeftButton:
            self._drawing = False

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.drawImage(0, 0, self.image)
        painter.end()


class PaintWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("SAGE Paint (Experimental)")
        self.canvas = Canvas()
        self.setCentralWidget(self.canvas)
        self._init_toolbar()
        self._show_disclaimer()

    def _init_toolbar(self):
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

        label = QLabel("EXPERIMENTAL", self)
        label.setStyleSheet("color: red; font-weight: bold;")
        toolbar.addWidget(label)

    def choose_color(self):
        color = QColorDialog.getColor(self.canvas.pen_color, self)
        if color.isValid():
            self.canvas.pen_color = color

    def increase_width(self):
        self.canvas.pen_width += 1

    def decrease_width(self):
        if self.canvas.pen_width > 1:
            self.canvas.pen_width -= 1

    def _show_disclaimer(self):
        QMessageBox.information(self, "Experimental", EXPERIMENTAL_NOTICE)


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    win = PaintWindow()
    win.show()
    return app.exec()
