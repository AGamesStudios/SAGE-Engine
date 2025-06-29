"""Simple yet extensible sprite painting application."""

from __future__ import annotations

import sys
import os
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QColorDialog,
    QToolBar, QLabel, QMessageBox, QSpinBox, QFileDialog
)
from PyQt6.QtGui import QAction, QActionGroup, QImage, QColor
from PyQt6.QtCore import Qt, QTimer, QPointF

from .canvas import Canvas
from .lang import LANGUAGES, DEFAULT_LANGUAGE
from sage.icons import load_icon

EXPERIMENTAL_NOTICE = (
    "SAGE Paint is experimental. Features may change and stability is not guaranteed."
)




class PaintWindow(QMainWindow):
    """Main window hosting a :class:`Canvas` and basic drawing tools."""

    def __init__(self, *, on_export=None, template: str | None = None,
                 project_dir: str | None = None, resource_dir: str | None = None):
        super().__init__()
        self.lang = DEFAULT_LANGUAGE
        self.setWindowTitle(self.t("SAGE Paint (Experimental)"))
        self.canvas = Canvas()
        self.setCentralWidget(self.canvas)
        self._first_show = True
        self.project_path: str | None = None
        self.project_dir = project_dir
        self.resource_dir = resource_dir
        self.on_export = on_export
        self._init_menu()
        self._init_toolbar()
        self._show_disclaimer()
        if template is not None:
            self.create_project(template)

    # ------------------------------------------------------------------
    def t(self, text: str) -> str:
        """Translate ``text`` for the current language."""
        return LANGUAGES.get(self.lang, LANGUAGES['English']).get(text, text)

    def set_language(self, lang: str) -> None:
        """Switch UI language and rebuild the interface."""
        if lang not in LANGUAGES:
            return
        self.lang = lang
        self.setWindowTitle(self.t("SAGE Paint (Experimental)"))
        self._init_menu()
        self._init_toolbar()

    # Menu ---------------------------------------------------------------

    def _init_menu(self) -> None:
        bar = self.menuBar()
        bar.clear()
        file_menu = bar.addMenu(self.t("File"))
        new_act = QAction(load_icon('file.png'), self.t("New"), self)
        new_act.setShortcut('Ctrl+N')
        new_act.triggered.connect(self.new_project)
        file_menu.addAction(new_act)
        open_act = QAction(load_icon('open.png'), self.t("Open..."), self)
        open_act.setShortcut('Ctrl+O')
        open_act.triggered.connect(self.open_project)
        file_menu.addAction(open_act)
        save_act = QAction(load_icon('save.png'), self.t("Save"), self)
        save_act.setShortcut('Ctrl+S')
        save_act.triggered.connect(self.save_project)
        file_menu.addAction(save_act)
        export_act = QAction(load_icon('folder.png'), self.t("Export PNG..."), self)
        export_act.setShortcut('Ctrl+E')
        export_act.triggered.connect(self.export_png)
        file_menu.addAction(export_act)

        settings = bar.addMenu(self.t("Settings"))
        size_action = QAction(load_icon('scale.png'), self.t("Set Window Size"), self)
        size_action.triggered.connect(self._change_size)
        settings.addAction(size_action)

        lang_menu = bar.addMenu(self.t("Language"))
        group = QActionGroup(self)
        for name in LANGUAGES:
            act = QAction(name, self)
            act.setCheckable(True)
            act.setChecked(name == self.lang)
            act.triggered.connect(lambda checked, n=name: self.set_language(n))
            group.addAction(act)
            lang_menu.addAction(act)

    def _change_size(self) -> None:
        from PyQt6.QtWidgets import QInputDialog
        w, ok = QInputDialog.getInt(
            self, self.t("Width"), self.t("Window width"), self.width(), 1)
        if not ok:
            return
        h, ok = QInputDialog.getInt(
            self, self.t("Height"), self.t("Window height"), self.height(), 1)
        if ok:
            self.resize(w, h)

    # Utility ------------------------------------------------------------

    def _maybe_continue(self) -> bool:
        if not self.canvas.is_dirty:
            return True
        resp = QMessageBox.question(
            self,
            self.t("Unsaved Changes"),
            self.t("Save changes before closing?"),
            QMessageBox.StandardButton.Save
            | QMessageBox.StandardButton.Discard
            | QMessageBox.StandardButton.Cancel,
        )
        if resp == QMessageBox.StandardButton.Save:
            self.save_project()
            return True
        return resp == QMessageBox.StandardButton.Discard

    # Project management -------------------------------------------------

    def new_project(self) -> None:
        if not self._maybe_continue():
            return
        from PyQt6.QtWidgets import QInputDialog

        templates = [
            self.t("White Background"),
            self.t("Dark Background"),
            self.t("Transparent"),
        ]
        choice, ok = QInputDialog.getItem(
            self,
            self.t("New Project"),
            self.t("Template"),
            templates,
            0,
            False,
        )
        if not ok:
            return
        if choice == self.t("Dark Background"):
            template = "dark"
        elif choice == self.t("Transparent"):
            template = "transparent"
        else:
            template = "white"
        self.create_project(template)

    def create_project(self, template: str = "white") -> None:
        """Create a project without prompting the user."""
        width = self.canvas.image.width()
        height = self.canvas.image.height()
        if template == "dark":
            bg = QColor(32, 32, 32)
        elif template == "transparent":
            bg = None
        else:
            bg = QColor("white")
        self.canvas.image = QImage(width, height, QImage.Format.Format_ARGB32)
        self.canvas.image.fill(Qt.GlobalColor.transparent)
        self.canvas.bg_color = bg
        self.canvas.undo_stack.clear()
        self.canvas.redo_stack.clear()
        self.canvas.center_on_image()
        self.project_path = None
        self.canvas.mark_clean()

    def open_project(self) -> None:
        if not self._maybe_continue():
            return
        path, _ = QFileDialog.getOpenFileName(
            self,
            self.t("Open"),
            '',
            "SAGE Paint (*.sagepaint);;Images (*.png *.jpg)"
        )
        if path:
            img = QImage(path)
            if not img.isNull():
                self.canvas.image = img
                self.canvas.bg_color = None
                self.canvas.center_on_image()
                self.project_path = path
                self.canvas.mark_clean()

    def save_project(self) -> None:
        path = self.project_path
        if not path:
            start = self.resource_dir or self.project_dir or ''
            path, _ = QFileDialog.getSaveFileName(
                self,
                self.t("Save"),
                start,
                "SAGE Paint (*.sagepaint)"
            )
        if path:
            if not path.endswith('.sagepaint'):
                path += '.sagepaint'
            self.canvas.image.save(path)
            self.project_path = path
            self.canvas.mark_clean()

    def export_png(self) -> None:
        start = self.project_dir or (
            os.path.dirname(self.project_path) if self.project_path else ''
        )
        path, _ = QFileDialog.getSaveFileName(
            self,
            self.t("Export"),
            start,
            "PNG Image (*.png)"
        )
        if path:
            if not path.endswith('.png'):
                path += '.png'
            self.canvas.image.save(path, 'PNG')
            if self.on_export:
                self.on_export(path)

    # Toolbar ------------------------------------------------------------

    def _init_toolbar(self) -> None:
        if hasattr(self, "tools_bar"):
            self.removeToolBar(self.tools_bar)
        if hasattr(self, "toolbar"):
            self.removeToolBar(self.toolbar)

        tools_bar = QToolBar(self)
        tools_bar.setOrientation(Qt.Orientation.Vertical)
        self.addToolBar(Qt.ToolBarArea.LeftToolBarArea, tools_bar)
        self.tools_bar = tools_bar
        group = QActionGroup(self)

        brush_action = QAction(load_icon('brush.png'), self.t("Brush"), self)
        brush_action.setShortcut('B')
        brush_action.setCheckable(True)
        brush_action.setChecked(True)
        brush_action.triggered.connect(lambda: self._select_tool('brush'))
        group.addAction(brush_action)
        tools_bar.addAction(brush_action)

        eraser_action = QAction(load_icon('eraser.png'), self.t("Eraser"), self)
        eraser_action.setShortcut('E')
        eraser_action.setCheckable(True)
        eraser_action.triggered.connect(lambda: self._select_tool('eraser'))
        group.addAction(eraser_action)
        tools_bar.addAction(eraser_action)

        fill_action = QAction(load_icon('fill.png'), self.t("Fill"), self)
        fill_action.setShortcut('F')
        fill_action.setCheckable(True)
        fill_action.triggered.connect(lambda: self._select_tool('fill'))
        group.addAction(fill_action)
        tools_bar.addAction(fill_action)

        select_action = QAction(load_icon('move.png'), self.t("Select"), self)
        select_action.setShortcut('M')
        select_action.setCheckable(True)
        select_action.triggered.connect(lambda: self._select_tool('select'))
        group.addAction(select_action)
        tools_bar.addAction(select_action)

        line_action = QAction(load_icon('pen.png'), self.t("Line"), self)
        line_action.setShortcut('L')
        line_action.setCheckable(True)
        line_action.triggered.connect(lambda: self._select_tool('line'))
        group.addAction(line_action)
        tools_bar.addAction(line_action)

        rect_action = QAction(load_icon('square.png'), self.t("Rect"), self)
        rect_action.setShortcut('R')
        rect_action.setCheckable(True)
        rect_action.triggered.connect(lambda: self._select_tool('rect'))
        group.addAction(rect_action)
        tools_bar.addAction(rect_action)

        self.brush_action = brush_action
        self.eraser_action = eraser_action
        self.fill_action = fill_action
        self.select_action = select_action
        self.line_action = line_action
        self.rect_action = rect_action

        toolbar = QToolBar(self)
        self.addToolBar(toolbar)
        self.toolbar = toolbar

        color_action = QAction(load_icon('colorpicker.png'), self.t("Color"), self)
        color_action.setShortcut('C')
        color_action.triggered.connect(self.choose_color)
        toolbar.addAction(color_action)

        from PyQt6.QtWidgets import QToolButton
        self.color_label = QToolButton()
        self.color_label.setFixedSize(24, 24)
        self.color_label.clicked.connect(self.choose_color)
        toolbar.addWidget(self.color_label)

        self.width_spin = QSpinBox(self)
        self.width_spin.setRange(1, 99)
        self.width_spin.setValue(self.canvas.pen_width)
        self.width_spin.valueChanged.connect(self.change_width)
        toolbar.addWidget(self.width_spin)

        toolbar.addSeparator()
        smooth_act = QAction(load_icon('smooth.png'), self.t("Smooth"), self)
        smooth_act.setCheckable(True)
        smooth_act.setChecked(True)
        smooth_act.triggered.connect(lambda checked: setattr(self.canvas, 'smooth_pen', checked))
        toolbar.addAction(smooth_act)

        toolbar.addSeparator()
        circle_act = QAction(load_icon('circle.png'), self.t("Circle"), self)
        circle_act.setCheckable(True)
        circle_act.setChecked(True)
        square_act = QAction(load_icon('square.png'), self.t("Square"), self)
        square_act.setCheckable(True)
        shape_group = QActionGroup(self)
        shape_group.addAction(circle_act)
        shape_group.addAction(square_act)
        circle_act.triggered.connect(lambda: self.canvas.set_brush_shape('circle'))
        square_act.triggered.connect(lambda: self.canvas.set_brush_shape('square'))
        toolbar.addAction(circle_act)
        toolbar.addAction(square_act)

        undo_act = QAction(load_icon('undo.png'), self.t("Undo"), self)
        undo_act.setShortcut('Ctrl+Z')
        undo_act.triggered.connect(self.canvas.undo)
        toolbar.addAction(undo_act)
        redo_act = QAction(load_icon('redo.png'), self.t("Redo"), self)
        redo_act.setShortcut('Ctrl+Y')
        redo_act.triggered.connect(self.canvas.redo)
        toolbar.addAction(redo_act)

        toolbar.addSeparator()
        zoom_in = QAction(load_icon('zoomin.png'), self.t("Zoom +"), self)
        zoom_in.setShortcut('=')
        zoom_in.triggered.connect(lambda: self.canvas.zoom_at(
            QPointF(self.canvas.rect().center()), 1.2
        ))
        toolbar.addAction(zoom_in)

        zoom_out = QAction(load_icon('zoomout.png'), self.t("Zoom -"), self)
        zoom_out.setShortcut('-')
        zoom_out.triggered.connect(lambda: self.canvas.zoom_at(
            QPointF(self.canvas.rect().center()), 1/1.2
        ))
        toolbar.addAction(zoom_out)

        label = QLabel(self.t("EXPERIMENTAL"), self)
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
        self.color_label.setStyleSheet(
            f"background-color: {c.name()}; border: 1px solid black;")

    def _select_tool(self, name: str) -> None:
        if name == 'brush':
            self.canvas.use_brush()
            self.width_spin.setValue(self.canvas.brush_width)
        elif name == 'eraser':
            self.canvas.use_eraser()
            self.width_spin.setValue(self.canvas.eraser_width)
        elif name == 'fill':
            self.canvas.use_fill()
        elif name == 'select':
            self.canvas.use_select()
        elif name == 'line':
            self.canvas.use_line()
            self.width_spin.setValue(self.canvas.brush_width)
        elif name == 'rect':
            self.canvas.use_rect()
            self.width_spin.setValue(self.canvas.brush_width)

    def set_pen_color(self, color: QColor) -> None:
        self.canvas.pen_color = color
        self._update_color_label()

    def change_width(self, value: int) -> None:
        self.canvas.set_pen_width(value)

    def _show_disclaimer(self) -> None:
        QMessageBox.information(self, self.t("Experimental"), self.t(EXPERIMENTAL_NOTICE))

    # Qt events ---------------------------------------------------------

    def showEvent(self, event) -> None:  # pragma: no cover - UI behavior
        super().showEvent(event)
        if self._first_show:
            self.showMaximized()
            QTimer.singleShot(0, self.canvas.center_on_image)
            self._first_show = False

    def closeEvent(self, event) -> None:  # pragma: no cover - UI behavior
        if self._maybe_continue():
            event.accept()
        else:
            event.ignore()


def main(argv: list[str] | None = None) -> int:
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    win = PaintWindow()
    win.show()
    return app.exec()
