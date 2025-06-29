"""Simple yet extensible sprite painting application."""

from __future__ import annotations

import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QColorDialog,
    QToolBar, QLabel, QMessageBox, QSpinBox, QFileDialog
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
        self.project_path: str | None = None
        self._init_menu()
        self._init_toolbar()
        self._show_disclaimer()

    # Menu ---------------------------------------------------------------

    def _init_menu(self) -> None:
        bar = self.menuBar()
        file_menu = bar.addMenu("File")
        new_act = QAction("New", self)
        new_act.triggered.connect(self.new_project)
        file_menu.addAction(new_act)
        open_act = QAction("Open...", self)
        open_act.triggered.connect(self.open_project)
        file_menu.addAction(open_act)
        save_act = QAction("Save", self)
        save_act.triggered.connect(self.save_project)
        file_menu.addAction(save_act)
        export_act = QAction("Export PNG...", self)
        export_act.triggered.connect(self.export_png)
        file_menu.addAction(export_act)

        settings = bar.addMenu("Settings")
        size_action = QAction("Set Window Size", self)
        size_action.triggered.connect(self._change_size)
        settings.addAction(size_action)

    def _change_size(self) -> None:
        from PyQt6.QtWidgets import QInputDialog
        w, ok = QInputDialog.getInt(self, "Width", "Window width", self.width(), 1)
        if not ok:
            return
        h, ok = QInputDialog.getInt(self, "Height", "Window height", self.height(), 1)
        if ok:
            self.resize(w, h)

    # Project management -------------------------------------------------

    def new_project(self) -> None:
        self.canvas.image.fill(QColor('white'))
        self.canvas.undo_stack.clear()
        self.canvas.redo_stack.clear()
        self.canvas.update()
        self.project_path = None

    def open_project(self) -> None:
        path, _ = QFileDialog.getOpenFileName(self, "Open", '', "SAGE Paint (*.sagepaint);;Images (*.png *.jpg)")
        if path:
            img = QImage(path)
            if not img.isNull():
                self.canvas.image = img
                self.canvas.update()
                self.project_path = path

    def save_project(self) -> None:
        path = self.project_path
        if not path:
            path, _ = QFileDialog.getSaveFileName(self, "Save", '', "SAGE Paint (*.sagepaint)")
        if path:
            if not path.endswith('.sagepaint'):
                path += '.sagepaint'
            self.canvas.image.save(path)
            self.project_path = path

    def export_png(self) -> None:
        path, _ = QFileDialog.getSaveFileName(self, "Export", '', "PNG Image (*.png)")
        if path:
            if not path.endswith('.png'):
                path += '.png'
            self.canvas.image.save(path, 'PNG')

    # Toolbar ------------------------------------------------------------

    def _init_toolbar(self) -> None:
        tools_bar = QToolBar(self)
        tools_bar.setOrientation(Qt.Orientation.Vertical)
        self.addToolBar(Qt.ToolBarArea.LeftToolBarArea, tools_bar)
        group = QActionGroup(self)

        brush_action = QAction(load_icon('brush.png'), "Brush", self)
        brush_action.setCheckable(True)
        brush_action.setChecked(True)
        brush_action.triggered.connect(lambda: self._select_tool('brush'))
        group.addAction(brush_action)
        tools_bar.addAction(brush_action)

        eraser_action = QAction(load_icon('eraser.png'), "Eraser", self)
        eraser_action.setCheckable(True)
        eraser_action.triggered.connect(lambda: self._select_tool('eraser'))
        group.addAction(eraser_action)
        tools_bar.addAction(eraser_action)

        fill_action = QAction(load_icon('fill.png'), "Fill", self)
        fill_action.setCheckable(True)
        fill_action.triggered.connect(lambda: self._select_tool('fill'))
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

        self.color_label = QLabel()
        self.color_label.setFixedSize(24, 24)
        toolbar.addWidget(self.color_label)

        self.width_spin = QSpinBox(self)
        self.width_spin.setRange(1, 99)
        self.width_spin.setValue(self.canvas.pen_width)
        self.width_spin.valueChanged.connect(self.change_width)
        toolbar.addWidget(self.width_spin)

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

        self._update_color_label()

    def choose_color(self) -> None:
        color = QColorDialog.getColor(self.canvas.pen_color, self)
        if color.isValid():
            self.set_pen_color(color)
            self.brush_action.setChecked(True)

    def _update_color_label(self) -> None:
        c = self.canvas.pen_color
        self.color_label.setStyleSheet(f"background-color: {c.name()}; border: 1px solid black;")

    def _select_tool(self, name: str) -> None:
        if name == 'brush':
            self.canvas.use_brush()
            self.width_spin.setValue(self.canvas.brush_width)
        elif name == 'eraser':
            self.canvas.use_eraser()
            self.width_spin.setValue(self.canvas.eraser_width)
        elif name == 'fill':
            self.canvas.use_fill()

    def set_pen_color(self, color: QColor) -> None:
        self.canvas.pen_color = color
        self._update_color_label()

    def change_width(self, value: int) -> None:
        self.canvas.set_pen_width(value)

    def _show_disclaimer(self) -> None:
        QMessageBox.information(self, "Experimental", EXPERIMENTAL_NOTICE)


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    win = PaintWindow()
    win.show()
    return app.exec()
