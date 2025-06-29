"""Simple yet extensible sprite painting application."""

from __future__ import annotations

import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QColorDialog,
    QToolBar, QLabel, QMessageBox
)
from PyQt6.QtGui import QAction, QActionGroup
from PyQt6.QtCore import Qt

from .canvas import Canvas
from sage_editor.icons import load_icon

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
        self._init_menu()
        self._init_toolbar()
        self._show_disclaimer()

    # Menu ---------------------------------------------------------------

    def _init_menu(self) -> None:
        menu = self.menuBar().addMenu("Settings")
        size_action = QAction("Set Window Size", self)
        size_action.triggered.connect(self._change_size)
        menu.addAction(size_action)

    def _change_size(self) -> None:
        from PyQt6.QtWidgets import QInputDialog
        w, ok = QInputDialog.getInt(self, "Width", "Window width", self.width(), 1)
        if not ok:
            return
        h, ok = QInputDialog.getInt(self, "Height", "Window height", self.height(), 1)
        if ok:
            self.resize(w, h)

    # Toolbar ------------------------------------------------------------

    def _init_toolbar(self) -> None:
        tools_bar = QToolBar(self)
        tools_bar.setOrientation(Qt.Orientation.Vertical)
        self.addToolBar(Qt.ToolBarArea.LeftToolBarArea, tools_bar)
        group = QActionGroup(self)
        brush_action = QAction(load_icon('brush.png'), "Brush", self)
        brush_action.setCheckable(True)
        brush_action.setChecked(True)
        brush_action.triggered.connect(lambda: self.canvas.use_brush())
        group.addAction(brush_action)
        tools_bar.addAction(brush_action)

        eraser_action = QAction(load_icon('eraser.png'), "Eraser", self)
        eraser_action.setCheckable(True)
        eraser_action.triggered.connect(lambda: self.canvas.use_eraser())
        group.addAction(eraser_action)
        tools_bar.addAction(eraser_action)

        fill_action = QAction(load_icon('fill.png'), "Fill", self)
        fill_action.setCheckable(True)
        fill_action.triggered.connect(lambda: self.canvas.use_fill())
        group.addAction(fill_action)
        tools_bar.addAction(fill_action)

        self.brush_action = brush_action
        self.eraser_action = eraser_action
        self.fill_action = fill_action

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

        toolbar.addSeparator()
        smooth_act = QAction("Smooth", self)
        smooth_act.setCheckable(True)
        smooth_act.setChecked(True)
        smooth_act.triggered.connect(lambda checked: setattr(self.canvas, 'smooth_pen', checked))
        toolbar.addAction(smooth_act)

        toolbar.addSeparator()
        circle_act = QAction("Circle", self)
        circle_act.setCheckable(True)
        circle_act.setChecked(True)
        square_act = QAction("Square", self)
        square_act.setCheckable(True)
        shape_group = QActionGroup(self)
        shape_group.addAction(circle_act)
        shape_group.addAction(square_act)
        circle_act.triggered.connect(lambda: self.canvas.set_brush_shape('circle'))
        square_act.triggered.connect(lambda: self.canvas.set_brush_shape('square'))
        toolbar.addAction(circle_act)
        toolbar.addAction(square_act)

        undo_act = QAction("Undo", self)
        undo_act.triggered.connect(self.canvas.undo)
        toolbar.addAction(undo_act)
        redo_act = QAction("Redo", self)
        redo_act.triggered.connect(self.canvas.redo)
        toolbar.addAction(redo_act)

        toolbar.addSeparator()
        zoom_in = QAction("Zoom +", self)
        zoom_in.triggered.connect(lambda: self.canvas.zoom_at(
            self.canvas.rect().center(), 1.2
        ))
        toolbar.addAction(zoom_in)

        zoom_out = QAction("Zoom -", self)
        zoom_out.triggered.connect(lambda: self.canvas.zoom_at(
            self.canvas.rect().center(), 1/1.2
        ))
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
