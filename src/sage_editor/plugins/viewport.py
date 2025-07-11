"""Qt editor window with dockable viewport, object list and console.

The widget preview uses :class:`~engine.renderers.opengl.core.OpenGLRenderer` so
objects are drawn with hardware acceleration by default.
"""

from __future__ import annotations

import logging
import os
from PyQt6.QtWidgets import (  # type: ignore[import-not-found]
    QApplication,
    QMainWindow,
    QListWidget,
    QDockWidget,
    QMenu,
    QPlainTextEdit,
    QMenuBar,
    QToolBar,
    QSizePolicy,
    QSplitter,
    QPushButton,
    QVBoxLayout,
    QGroupBox,
    QFormLayout,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QSlider,
    QCheckBox,
    QStyleFactory,
)
from PyQt6.QtGui import QAction  # type: ignore[import-not-found]
from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
from typing import Optional, cast
from PyQt6.QtWidgets import QWidget  # type: ignore[import-not-found]
from engine.utils import units

from engine.renderers.opengl.core import OpenGLRenderer
from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from engine import gizmos
from engine.core.objects import get_object_type

from sage_editor.qt import GLWidget, SDLWidget

log = logging.getLogger(__name__)


def _apply_dark_palette(app: QApplication) -> None:
    """Apply a dark Fusion palette to *app*."""
    from PyQt6.QtGui import QColor, QPalette  # type: ignore[import-not-found]

    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.WindowText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Base, QColor(42, 42, 42))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(66, 66, 66))
    palette.setColor(QPalette.ColorRole.ToolTipBase, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.ToolTipText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Text, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ButtonText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.BrightText, Qt.GlobalColor.red)
    palette.setColor(QPalette.ColorRole.Link, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.HighlightedText, Qt.GlobalColor.black)
    app.setPalette(palette)


class _ViewportMixin:
    """Input helpers shared between OpenGL and SDL widgets."""

    DRAG_THRESHOLD = 5

    def __init__(self, window: "EditorWindow", *a, **k) -> None:
        """Initialise mixin state without touching QWidget internals."""
        self._window = window
        self._last_pos = None
        self._press_pos = None
        self._dragging = False
        if hasattr(self, "setContextMenuPolicy"):
            cast(QWidget, self).setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        if hasattr(self, "customContextMenuRequested"):
            cast(QWidget, self).customContextMenuRequested.connect(self._context_menu)

    # event handlers are same as before
    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            self._last_pos = ev.position() if hasattr(ev, "position") else ev.pos()
            self._press_pos = self._last_pos
            self._dragging = False
            try:
                from PyQt6.QtGui import QCursor  # type: ignore[import-not-found]
                cast(QWidget, self).setCursor(QCursor(Qt.CursorShape.ClosedHandCursor))
            except Exception:
                log.exception("Failed to set cursor")

    def mouseMoveEvent(self, ev):  # pragma: no cover - gui interaction
        if self._press_pos is not None and ev.buttons() & Qt.MouseButton.LeftButton:
            pos = ev.position() if hasattr(ev, "position") else ev.pos()
            if not self._dragging:
                dx = pos.x() - self._press_pos.x()
                dy = pos.y() - self._press_pos.y()
                if abs(dx) > self.DRAG_THRESHOLD or abs(dy) > self.DRAG_THRESHOLD:
                    self._dragging = True
                    self._last_pos = pos
            if self._dragging and self._last_pos is not None:
                dx = pos.x() - self._last_pos.x()
                dy = pos.y() - self._last_pos.y()
                cam = self._window.camera
                scale = units.UNITS_PER_METER
                sign = -1 if units.Y_UP else 1
                cam.x -= dx / (scale * cam.zoom)
                cam.y -= dy * sign / (scale * cam.zoom)
                self._window.draw_scene(update_list=False)
                self._last_pos = pos

    def mouseReleaseEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            pos = ev.position() if hasattr(ev, "position") else ev.pos()
            if self._press_pos is not None and not self._dragging:
                wx, wy = self._window.screen_to_world(pos)
                obj = self._window.find_object_at(wx, wy)
                self._window.select_object(obj)
            self._press_pos = None
            self._dragging = False
            self._last_pos = None
            try:
                cast(QWidget, self).unsetCursor()
            except Exception:
                log.exception("Failed to unset cursor")

    def wheelEvent(self, ev):  # pragma: no cover - gui interaction
        delta = 0
        if hasattr(ev, "angleDelta"):
            delta = ev.angleDelta().y()
        elif hasattr(ev, "delta"):
            delta = ev.delta()
        if delta:
            factor = 1.1 ** (delta / 120)
            cam = self._window.camera
            cam.zoom *= factor
            if cam.zoom <= 0:
                cam.zoom = 0.1
            self._window.draw_scene(update_list=False)

    def _context_menu(self, point):  # pragma: no cover - gui interaction
        menu = QMenu(self)
        wx, wy = self._window.screen_to_world(point)
        obj = self._window.find_object_at(wx, wy)
        if obj is not None:
            self._window.select_object(obj)
            copy_a = menu.addAction("Copy")
            paste_a = menu.addAction("Paste")
            del_a = menu.addAction("Delete")
            if copy_a is not None and hasattr(copy_a, "triggered"):
                copy_a.triggered.connect(self._window.copy_selected)
            if paste_a is not None and hasattr(paste_a, "triggered"):
                paste_a.triggered.connect(self._window.paste_object)
            if del_a is not None and hasattr(del_a, "triggered"):
                del_a.triggered.connect(self._window.delete_selected)
        else:
            action = menu.addAction("Create Object")

            def cb():
                self._window.create_object(wx, wy)

            if action is not None and hasattr(action, "triggered"):
                action.triggered.connect(cb)
        menu.exec(cast(QWidget, self).mapToGlobal(point))


class ViewportWidget(_ViewportMixin, GLWidget):
    """OpenGL viewport widget."""

    def __init__(self, window: "EditorWindow") -> None:
        GLWidget.__init__(self, window)
        _ViewportMixin.__init__(self, window)


class SDLViewportWidget(_ViewportMixin, SDLWidget):
    """SDL viewport widget."""

    def __init__(self, window: "EditorWindow") -> None:
        SDLWidget.__init__(self, window)
        _ViewportMixin.__init__(self, window)

    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            self._last_pos = ev.position() if hasattr(ev, "position") else ev.pos()
            self._press_pos = self._last_pos
            self._dragging = False
            try:
                from PyQt6.QtGui import QCursor  # type: ignore[import-not-found]
                cast(QWidget, self).setCursor(QCursor(Qt.CursorShape.ClosedHandCursor))
            except Exception:
                log.exception("Failed to set cursor")

    def mouseMoveEvent(self, ev):  # pragma: no cover - gui interaction
        if self._press_pos is not None and ev.buttons() & Qt.MouseButton.LeftButton:
            pos = ev.position() if hasattr(ev, "position") else ev.pos()
            if not self._dragging:
                dx = pos.x() - self._press_pos.x()
                dy = pos.y() - self._press_pos.y()
                if abs(dx) > self.DRAG_THRESHOLD or abs(dy) > self.DRAG_THRESHOLD:
                    self._dragging = True
                    self._last_pos = pos
            if self._dragging and self._last_pos is not None:
                dx = pos.x() - self._last_pos.x()
                dy = pos.y() - self._last_pos.y()
                cam = self._window.camera
                scale = units.UNITS_PER_METER
                sign = -1 if units.Y_UP else 1
                cam.x -= dx / (scale * cam.zoom)
                cam.y -= dy * sign / (scale * cam.zoom)
                self._window.draw_scene(update_list=False)
                self._last_pos = pos

    def mouseReleaseEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            pos = ev.position() if hasattr(ev, "position") else ev.pos()
            if self._press_pos is not None and not self._dragging:
                wx, wy = self._window.screen_to_world(pos)
                obj = self._window.find_object_at(wx, wy)
                self._window.select_object(obj)
            self._press_pos = None
            self._dragging = False
            self._last_pos = None
            try:
                cast(QWidget, self).unsetCursor()
            except Exception:
                log.exception("Failed to unset cursor")

    def wheelEvent(self, ev):  # pragma: no cover - gui interaction
        delta = 0
        if hasattr(ev, "angleDelta"):
            delta = ev.angleDelta().y()
        elif hasattr(ev, "delta"):
            delta = ev.delta()
        if delta:
            factor = 1.1 ** (delta / 120)
            cam = self._window.camera
            cam.zoom *= factor
            if cam.zoom <= 0:
                cam.zoom = 0.1
            self._window.draw_scene(update_list=False)

    def _context_menu(self, point):  # pragma: no cover - gui interaction
        menu = QMenu(self)
        wx, wy = self._window.screen_to_world(point)
        obj = self._window.find_object_at(wx, wy)
        if obj is not None:
            self._window.select_object(obj)
            copy_a = menu.addAction("Copy")
            paste_a = menu.addAction("Paste")
            del_a = menu.addAction("Delete")
            if copy_a is not None and hasattr(copy_a, "triggered"):
                copy_a.triggered.connect(self._window.copy_selected)
            if paste_a is not None and hasattr(paste_a, "triggered"):
                paste_a.triggered.connect(self._window.paste_object)
            if del_a is not None and hasattr(del_a, "triggered"):
                del_a.triggered.connect(self._window.delete_selected)
        else:
            action = menu.addAction("Create Object")

            def cb():
                self._window.create_object(wx, wy)

            if action is not None and hasattr(action, "triggered"):
                action.triggered.connect(cb)
        menu.exec(cast(QWidget, self).mapToGlobal(point))


class PropertiesWidget(QWidget):
    """Widget for editing object properties."""

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)

        self.object_group = QGroupBox("Object", self)
        obj_form = QFormLayout(self.object_group)
        self.name_edit = QLineEdit(self)
        obj_form.addRow("Name", self.name_edit)
        self.type_edit = QLineEdit(self)
        self.type_edit.setReadOnly(True)
        obj_form.addRow("Type", self.type_edit)
        self.tags_edit = QLineEdit(self)
        obj_form.addRow("Tags", self.tags_edit)
        self.visible_check = QCheckBox("Visible", self)
        obj_form.addRow(self.visible_check)
        layout.addWidget(self.object_group)

        self.transform_group = QGroupBox("Transform", self)
        trans_form = QFormLayout(self.transform_group)
        pos_widget = QWidget(self)
        pos_layout = QHBoxLayout(pos_widget)
        self.pos_x = QLineEdit(self)
        self.pos_y = QLineEdit(self)
        pos_layout.addWidget(QLabel("X", self))
        pos_layout.addWidget(self.pos_x)
        pos_layout.addWidget(QLabel("Y", self))
        pos_layout.addWidget(self.pos_y)
        trans_form.addRow("Position", pos_widget)

        self.rot_slider = QSlider(Qt.Orientation.Horizontal, self)
        self.rot_slider.setRange(0, 360)
        trans_form.addRow("Rotation", self.rot_slider)

        scale_widget = QWidget(self)
        scale_layout = QHBoxLayout(scale_widget)
        self.scale_x = QLineEdit(self)
        self.scale_y = QLineEdit(self)
        self.link_scale = QCheckBox("Link", self)
        scale_layout.addWidget(QLabel("X", self))
        scale_layout.addWidget(self.scale_x)
        scale_layout.addWidget(QLabel("Y", self))
        scale_layout.addWidget(self.scale_y)
        scale_layout.addWidget(self.link_scale)
        trans_form.addRow("Scale", scale_widget)

        pivot_widget = QWidget(self)
        pivot_layout = QHBoxLayout(pivot_widget)
        self.pivot_x = QLineEdit(self)
        self.pivot_y = QLineEdit(self)
        pivot_layout.addWidget(QLabel("X", self))
        pivot_layout.addWidget(self.pivot_x)
        pivot_layout.addWidget(QLabel("Y", self))
        pivot_layout.addWidget(self.pivot_y)
        trans_form.addRow("Pivot", pivot_widget)

        flip_widget = QWidget(self)
        flip_layout = QHBoxLayout(flip_widget)
        self.flip_x = QCheckBox("X", self)
        self.flip_y = QCheckBox("Y", self)
        flip_layout.addWidget(self.flip_x)
        flip_layout.addWidget(self.flip_y)
        trans_form.addRow("Flip", flip_widget)

        self.scale_x.editingFinished.connect(self._sync_scale_x)
        self.scale_y.editingFinished.connect(self._sync_scale_y)

        layout.addWidget(self.transform_group)

        self.apply_btn = QPushButton("Apply", self)
        layout.addWidget(self.apply_btn)
        layout.addStretch()

    def set_object(self, obj: Optional[GameObject]) -> None:
        if obj is None:
            self.name_edit.setText("")
            self.type_edit.setText("")
            self.tags_edit.setText("")
            self.visible_check.setChecked(False)
            self.pos_x.setText("")
            self.pos_y.setText("")
            self.rot_slider.setValue(0)
            self.scale_x.setText("")
            self.scale_y.setText("")
            self.flip_x.setChecked(False)
            self.flip_y.setChecked(False)
            self.pivot_x.setText("")
            self.pivot_y.setText("")
            return

        self.name_edit.setText(obj.name or "")
        typ = (
            getattr(obj, "role", None)
            or getattr(obj, "type", None)
            or get_object_type(obj)
            or type(obj).__name__
        )
        self.type_edit.setText(str(typ))
        tags = obj.metadata.get("tags", [])
        if isinstance(tags, (list, set)):
            tags = ",".join(tags)
        self.tags_edit.setText(str(tags))
        self.visible_check.setChecked(bool(getattr(obj, "visible", True)))
        self.pos_x.setText(str(getattr(obj, "x", 0.0)))
        self.pos_y.setText(str(getattr(obj, "y", 0.0)))
        self.rot_slider.setValue(int(getattr(obj, "angle", 0.0) % 360))
        self.scale_x.setText(str(getattr(obj, "scale_x", 1.0)))
        self.scale_y.setText(str(getattr(obj, "scale_y", 1.0)))
        self.flip_x.setChecked(bool(getattr(obj, "flip_x", False)))
        self.flip_y.setChecked(bool(getattr(obj, "flip_y", False)))
        self.pivot_x.setText(str(getattr(obj, "pivot_x", 0.0)))
        self.pivot_y.setText(str(getattr(obj, "pivot_y", 0.0)))

    def apply_to_object(self, obj: GameObject) -> None:
        obj.name = self.name_edit.text()
        tags = [t.strip() for t in self.tags_edit.text().split(',') if t.strip()]
        if tags:
            obj.metadata["tags"] = tags
        obj.visible = self.visible_check.isChecked()
        try:
            obj.x = float(self.pos_x.text())
            obj.y = float(self.pos_y.text())
        except ValueError:
            log.warning("Invalid position")
        obj.angle = float(self.rot_slider.value())
        try:
            sx = float(self.scale_x.text())
            sy = float(self.scale_y.text())
            if self.link_scale.isChecked():
                sy = sx
            obj.scale_x = sx
            obj.scale_y = sy
        except ValueError:
            log.warning("Invalid scale")
        obj.flip_x = self.flip_x.isChecked()
        obj.flip_y = self.flip_y.isChecked()
        try:
            obj.pivot_x = float(self.pivot_x.text())
            obj.pivot_y = float(self.pivot_y.text())
        except ValueError:
            log.warning("Invalid pivot")

    def _sync_scale_x(self):
        if self.link_scale.isChecked():
            self.scale_y.setText(self.scale_x.text())

    def _sync_scale_y(self):
        if self.link_scale.isChecked():
            self.scale_x.setText(self.scale_y.text())


class EditorWindow(QMainWindow):
    """Main editor window with dockable widgets."""

    def _create_viewport_widget(self, backend: str):
        if backend == "sdl":
            return SDLViewportWidget(self)
        return ViewportWidget(self)

    def __init__(self, menus=None, toolbar=None, *, backend: str = "opengl") -> None:
        super().__init__()
        self.setWindowTitle("SAGE Editor")

        self.setDockNestingEnabled(True)

        self._engine = None
        self._game_window = None
        self.viewport = self._create_viewport_widget(backend)
        self.console = QPlainTextEdit(self)
        self.properties = PropertiesWidget(self)
        self.prop_apply = self.properties.apply_btn
        self.resources = QListWidget()

        # minimal scene used for previewing objects
        w = self.viewport.width() or 640
        h = self.viewport.height() or 480
        if backend == "opengl":
            rcls = OpenGLRenderer
        else:
            from engine.renderers import get_renderer
            rcls = get_renderer(backend)
            if rcls is None:
                self.log_warning(f"Renderer '{backend}' unavailable; falling back to OpenGL")
                rcls = OpenGLRenderer
        self.renderer_backend = backend if rcls is not OpenGLRenderer else "opengl"
        self.renderer = rcls(
            width=w,
            height=h,
            widget=self.viewport,
            vsync=False,
            keep_aspect=False,
        )
        self.camera = Camera(width=w, height=h, active=True)
        self.scene = Scene(with_defaults=False)
        self.project_path: str | None = None
        # keep the viewport camera separate from scene objects
        self.renderer.show_grid = True
        self.set_renderer(self.renderer)
        self.selected_obj: Optional[GameObject] = None
        self._clipboard: dict | None = None

        splitter = QSplitter(Qt.Orientation.Vertical, self)
        splitter.addWidget(self.viewport)
        splitter.addWidget(self.console)
        splitter.setStretchFactor(0, 5)
        splitter.setStretchFactor(1, 1)
        self._splitter = splitter
        self.setCentralWidget(splitter)

        menubar = QMenuBar(self)
        self.setMenuBar(menubar)
        file_menu = menubar.addMenu("File")
        open_p = file_menu.addAction("Open Project")
        save_p = file_menu.addAction("Save Project")
        shot_p = file_menu.addAction("Screenshot...")
        if open_p is not None and hasattr(open_p, "triggered"):
            open_p.triggered.connect(self.open_project_dialog)
        if save_p is not None and hasattr(save_p, "triggered"):
            save_p.triggered.connect(self.save_project_dialog)
        if shot_p is not None and hasattr(shot_p, "triggered"):
            shot_p.triggered.connect(self.open_screenshot_dialog)
        renderer_menu = menubar.addMenu("Renderer")
        ogl_action = renderer_menu.addAction("OpenGL")
        sdl_action = renderer_menu.addAction("SDL")
        if ogl_action is not None and hasattr(ogl_action, "triggered"):
            ogl_action.triggered.connect(lambda: self.change_renderer("opengl"))
        if sdl_action is not None and hasattr(sdl_action, "triggered"):
            sdl_action.triggered.connect(lambda: self.change_renderer("sdl"))
        view_menu = menubar.addMenu("View")
        grid_act = view_menu.addAction("Show Grid")
        if grid_act is not None:
            if hasattr(grid_act, "setCheckable"):
                grid_act.setCheckable(True)
            if hasattr(grid_act, "setChecked"):
                grid_act.setChecked(True)
            if hasattr(grid_act, "triggered"):
                grid_act.triggered.connect(self.toggle_grid)

        if menus:
            for title, cb in menus:
                action = QAction(title, self)
                if action is not None and hasattr(action, "triggered"):
                    action.triggered.connect(cb)
                menubar.addAction(action)

        tbar = QToolBar(self)
        self.addToolBar(tbar)
        left_spacer = QWidget(self)
        left_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(left_spacer)

        run_action = QAction("Run", self)
        if run_action is not None and hasattr(run_action, "triggered"):
            run_action.triggered.connect(self.start_game)
        tbar.addAction(run_action)

        shot_action = QAction("Screenshot", self)
        if shot_action is not None:
            if hasattr(shot_action, "triggered"):
                shot_action.triggered.connect(self.open_screenshot_dialog)
            tbar.addAction(shot_action)

        right_spacer = QWidget(self)
        right_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(right_spacer)
        if toolbar:
            for title, cb in toolbar:
                action = QAction(title, self)
                if action is not None and hasattr(action, "triggered"):
                    action.triggered.connect(cb)
                tbar.addAction(action)

        self.objects = QListWidget()
        obj_dock = QDockWidget("Objects", self)
        obj_dock.setObjectName("ObjectsDock")
        obj_dock.setWidget(self.objects)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)

        prop_dock = QDockWidget("Properties", self)
        prop_dock.setObjectName("PropertiesDock")
        prop_dock.setWidget(self.properties)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, prop_dock)
        self.splitDockWidget(obj_dock, prop_dock, Qt.Orientation.Vertical)

        res_dock = QDockWidget("Resources", self)
        res_dock.setObjectName("ResourcesDock")
        res_dock.setWidget(self.resources)
        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, res_dock)

        self.objects.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.objects.customContextMenuRequested.connect(self._list_context_menu)
        self.objects.currentItemChanged.connect(self._object_selected)

        self.selected_obj = None
        self.update_object_list()
        self.prop_apply.clicked.connect(self.apply_properties)
        self.draw_scene()

    def log_warning(self, text: str) -> None:
        """Display *text* in the console dock."""
        self.console.appendPlainText(text)

    def toggle_grid(self, checked: bool) -> None:
        if hasattr(self.renderer, "show_grid"):
            self.renderer.show_grid = checked
            self.draw_scene(update_list=False)

    def save_screenshot(self, path: str) -> None:
        if self.renderer is None:
            return
        try:
            self.renderer.save_screenshot(path)
            self.log_warning(f"Screenshot saved to {path}")
        except Exception:
            log.exception("Screenshot failed")

    def open_screenshot_dialog(self) -> None:
        if self.renderer is None:
            return
        from PyQt6.QtWidgets import (
            QDialog,
            QFormLayout,
            QLineEdit,
            QCheckBox,
            QPushButton,
            QFileDialog,
            QHBoxLayout,
        )  # type: ignore[import-not-found]

        dlg = QDialog(self)
        dlg.setWindowTitle("Save Screenshot")
        form = QFormLayout(dlg)

        path_edit = QLineEdit("screenshot.png", dlg)
        browse_btn = QPushButton("Browse", dlg)

        def browse() -> None:
            path, _ = QFileDialog.getSaveFileName(
                self,
                "Save Screenshot",
                path_edit.text(),
                "PNG Images (*.png)",
            )
            if path:
                path_edit.setText(path)

        browse_btn.clicked.connect(browse)
        path_row = QHBoxLayout()
        path_row.addWidget(path_edit)
        path_row.addWidget(browse_btn)
        form.addRow("Path", path_row)

        grid_check = QCheckBox("Show Grid", dlg)
        if hasattr(self.renderer, "show_grid"):
            grid_check.setChecked(self.renderer.show_grid)
        form.addRow(grid_check)

        btn_row = QHBoxLayout()
        ok_btn = QPushButton("Save", dlg)
        cancel_btn = QPushButton("Cancel", dlg)
        ok_btn.clicked.connect(dlg.accept)
        cancel_btn.clicked.connect(dlg.reject)
        btn_row.addWidget(ok_btn)
        btn_row.addWidget(cancel_btn)
        form.addRow(btn_row)

        if dlg.exec():
            path = path_edit.text()
            if not path:
                return
            show_grid = grid_check.isChecked()
            orig = getattr(self.renderer, "show_grid", True)
            if hasattr(self.renderer, "show_grid"):
                self.renderer.show_grid = show_grid
            self.draw_scene(update_list=False)
            try:
                self.save_screenshot(path)
            finally:
                if hasattr(self.renderer, "show_grid"):
                    self.renderer.show_grid = orig
                self.draw_scene(update_list=False)

    def load_project(self, path: str) -> None:
        from engine.core.project import Project
        from engine.core.resources import set_resource_root

        proj = Project.load(path)
        base = os.path.dirname(path)
        set_resource_root(os.path.join(base, proj.resources))
        self.scene = Scene.from_dict(proj.scene, base_path=base)
        self.camera = self.scene.ensure_active_camera(proj.width, proj.height)
        if hasattr(self.renderer, "set_window_size"):
            self.renderer.set_window_size(proj.width, proj.height)
        self.project_path = path
        self.update_object_list()
        self.draw_scene()

    def save_project(self, path: str) -> None:
        from engine.core.project import Project

        proj = Project(
            scene=self.scene.to_dict(),
            renderer=self.renderer_backend,
            width=self.camera.width,
            height=self.camera.height,
            keep_aspect=self.renderer.keep_aspect,
        )
        proj.save(path)
        self.project_path = path

    def open_project_dialog(self) -> None:
        from PyQt6.QtWidgets import QFileDialog  # type: ignore[import-not-found]

        path, _ = QFileDialog.getOpenFileName(
            self,
            "Open Project",
            "",
            "SAGE Project (*.sageproject)",
        )
        if path:
            try:
                self.load_project(path)
                self.log_warning(f"Loaded project {path}")
            except Exception:
                log.exception("Failed to load project")

    def save_project_dialog(self) -> None:
        from PyQt6.QtWidgets import QFileDialog  # type: ignore[import-not-found]

        path, _ = QFileDialog.getSaveFileName(
            self,
            "Save Project",
            self.project_path or "project.sageproject",
            "SAGE Project (*.sageproject)",
        )
        if path:
            try:
                self.save_project(path)
                self.log_warning(f"Project saved to {path}")
            except Exception:
                log.exception("Failed to save project")

    # coordinate helpers -------------------------------------------------

    def screen_to_world(self, point):
        w = self.viewport.width() or 1
        h = self.viewport.height() or 1
        cam = self.camera
        scale = units.UNITS_PER_METER
        sign = -1 if units.Y_UP else 1
        x = cam.x + (point.x() - w / 2) / (scale * cam.zoom)
        y = cam.y + (point.y() - h / 2) * sign / (scale * cam.zoom)
        return x, y

    def set_renderer(self, renderer):
        self.viewport.renderer = renderer

    def change_renderer(self, backend: str) -> None:
        if backend == "opengl":
            rcls = OpenGLRenderer
        else:
            from engine.renderers import get_renderer
            rcls = get_renderer(backend)
            if rcls is None:
                self.log_warning(f"Renderer '{backend}' unavailable; falling back to OpenGL")
                rcls = OpenGLRenderer
        if self.renderer is not None:
            try:
                self.renderer.close()
            except Exception:
                log.exception("Renderer close failed")
        old_view = self.viewport
        old_splitter = self._splitter
        w = self.viewport.width() or 640
        h = self.viewport.height() or 480
        if (backend == "sdl" and not isinstance(self.viewport, SDLViewportWidget)) or (
            backend != "sdl" and isinstance(self.viewport, SDLViewportWidget)
        ):
            if old_view is not None and hasattr(old_view, "deleteLater"):
                old_view.deleteLater()
            if old_splitter is not None and hasattr(old_splitter, "deleteLater"):
                old_splitter.deleteLater()
            new_view = self._create_viewport_widget(backend)
            splitter = QSplitter(Qt.Orientation.Vertical, self)
            splitter.addWidget(new_view)
            splitter.addWidget(self.console)
            splitter.setStretchFactor(0, 5)
            splitter.setStretchFactor(1, 1)
            self._splitter = splitter
            self.setCentralWidget(splitter)
            self.viewport = new_view
        self.renderer = rcls(width=w, height=h, widget=self.viewport, vsync=False, keep_aspect=False)
        self.renderer_backend = backend if rcls is not OpenGLRenderer else "opengl"
        self.set_renderer(self.renderer)
        self.draw_scene()

    def set_objects(self, names):
        self.objects.clear()
        self.objects.addItems(list(names))

    def update_object_list(self, preserve: bool = True):
        current = self.selected_obj.name if preserve and self.selected_obj else None
        names = [obj.name for obj in self.scene.objects]
        self.set_objects(names)
        if current:
            for i in range(self.objects.count()):
                item = self.objects.item(i)
                if item.text() == current:
                    self.objects.setCurrentItem(item)
                    break

    def update_properties(self):
        self.properties.set_object(self.selected_obj)

    def apply_properties(self):
        if self.selected_obj is None:
            return
        try:
            self.properties.apply_to_object(self.selected_obj)
        except Exception:
            log.exception("Failed to apply properties")
        self.draw_scene()

    def find_object_at(self, x: float, y: float) -> Optional[GameObject]:
        for obj in reversed(self.scene.objects):
            if isinstance(obj, Camera):
                continue
            left, bottom, w, h = obj.rect()
            if left <= x <= left + w and bottom <= y <= bottom + h:
                return obj
        return None

    def select_object(self, obj: Optional[GameObject]) -> None:
        self.selected_obj = obj
        if obj is None:
            self.objects.setCurrentItem(None)
        else:
            for i in range(self.objects.count()):
                item = self.objects.item(i)
                if item.text() == obj.name:
                    self.objects.setCurrentItem(item)
                    break
        self.update_properties()
        self.draw_scene(update_list=False)

    def _update_selection_gizmo(self) -> None:
        """Refresh gizmo highlighting the currently selected object."""
        if hasattr(self.renderer, "clear_gizmos"):
            self.renderer.clear_gizmos()
        obj = self.selected_obj
        if obj is None:
            return
        if isinstance(obj, Camera):
            left, bottom, w, h = obj.view_rect()
        else:
            left, bottom, w, h = obj.rect()
        points = [
            (left, bottom),
            (left + w, bottom),
            (left + w, bottom + h),
            (left, bottom + h),
            (left, bottom),
        ]
        g = gizmos.polyline_gizmo(points, color=(1, 0, 0, 1), frames=None)
        if hasattr(self.renderer, "add_gizmo"):
            self.renderer.add_gizmo(g)
            self.renderer.add_gizmo(
                gizmos.circle_gizmo(
                    obj.x,
                    obj.y,
                    size=4,
                    color=(0.5, 0.5, 0.5, 1),
                    thickness=1,
                    frames=None,
                )
            )

    def draw_scene(self, update_list: bool = True) -> None:
        """Render the current scene and refresh selection gizmos."""
        self._update_selection_gizmo()
        if update_list:
            self.update_object_list()
        self.renderer.draw_scene(self.scene, self.camera)

    def start_game(self):
        from engine.core.engine import Engine
        from engine.game_window import GameWindow
        self.close_game()
        w = self.renderer.width
        h = self.renderer.height
        self._engine = Engine(
            width=w,
            height=h,
            scene=self.scene,
            camera=self.camera,
            renderer=self.renderer_backend,
            keep_aspect=self.renderer.keep_aspect,
        )
        self._game_window = GameWindow(self._engine)
        self._game_window.closed.connect(self.close_game)
        self._game_window.show()

    def close_game(self):
        """Close the running game window and shut down its engine."""
        if self._game_window is not None:
            win = self._game_window
            self._game_window = None
            try:
                win.closed.disconnect(self.close_game)
            except Exception:
                pass
            try:
                win.close()
            except Exception:
                log.exception("Failed to close game window")
        if self._engine is not None:
            try:
                self._engine.shutdown()
                if hasattr(self._engine, "renderer"):
                    self._engine.renderer.close()
            except Exception:
                log.exception("Failed to shut down engine")
            self._engine = None

    def create_object(self, x: float = 0.0, y: float = 0.0) -> GameObject:
        count = len([o for o in self.scene.objects if not isinstance(o, Camera)])
        obj = GameObject(name=f"Object {count}")
        obj.transform.x = x
        obj.transform.y = y
        self.scene.add_object(obj)
        self.update_object_list()
        self.draw_scene()
        return obj

    # clipboard and selection helpers ----------------------------------

    def copy_selected(self) -> None:
        if self.selected_obj is None:
            return
        from engine.core.objects import object_to_dict
        self._clipboard = object_to_dict(self.selected_obj)

    def paste_object(self) -> Optional[GameObject]:
        if not self._clipboard:
            return None
        from engine.core.objects import object_from_dict
        try:
            obj = cast(GameObject, object_from_dict(dict(self._clipboard)))
        except Exception:
            log.exception("Failed to paste object")
            return None
        base = obj.name
        idx = 1
        while self.scene.find_object(obj.name):
            obj.name = f"{base}_{idx}"
            idx += 1
        self.scene.add_object(obj)
        self.update_object_list()
        self.select_object(obj)
        return obj

    def delete_selected(self) -> None:
        if self.selected_obj is None:
            return
        self.scene.remove_object(self.selected_obj)
        self.selected_obj = None
        self.update_object_list()
        self.draw_scene()

    def _list_context_menu(self, point):
        menu = QMenu(self.objects)
        if self.selected_obj is not None:
            copy_a = menu.addAction("Copy")
            paste_a = menu.addAction("Paste")
            del_a = menu.addAction("Delete")
            if copy_a is not None and hasattr(copy_a, "triggered"):
                copy_a.triggered.connect(self.copy_selected)
            if paste_a is not None and hasattr(paste_a, "triggered"):
                paste_a.triggered.connect(self.paste_object)
            if del_a is not None and hasattr(del_a, "triggered"):
                del_a.triggered.connect(self.delete_selected)
            menu.addSeparator()
        action = menu.addAction("Create Object")
        if action is not None and hasattr(action, "triggered"):
            action.triggered.connect(self.create_object)
        menu.exec(self.objects.mapToGlobal(point))

    def _object_selected(self, current, _prev):
        if current is not None:
            name = current.text()
            self.selected_obj = self.scene.find_object(name)
        else:
            self.selected_obj = None
        self.update_properties()
        self.draw_scene(update_list=False)

    def closeEvent(self, event):  # pragma: no cover - gui cleanup
        if self.renderer is not None:
            try:
                self.renderer.close()
            except Exception:
                log.exception("Renderer close failed")
        self.close_game()
        super().closeEvent(event)


def init_editor(editor) -> None:
    """Launch the main editor window and attach it to *editor*."""
    app = QApplication.instance()
    created = False
    if app is None:
        from PyQt6.QtCore import QCoreApplication, Qt
        attr = getattr(Qt.ApplicationAttribute, "AA_EnableHighDpiScaling", None)
        if attr is not None:
            QCoreApplication.setAttribute(attr, True)
        attr = getattr(Qt.ApplicationAttribute, "AA_UseHighDpiPixmaps", None)
        if attr is not None:
            QCoreApplication.setAttribute(attr, True)
        app = QApplication([])
        created = True
        font = app.font()
        if font.pointSize() < 10:
            font.setPointSize(10)
            app.setFont(font)
    if hasattr(QApplication, "setStyle"):
        QApplication.setStyle(QStyleFactory.create("Fusion"))
        _apply_dark_palette(app)

    window = EditorWindow(editor._menus, editor._toolbar)
    window.resize(800, 600)
    window.show()

    editor.window = window
    editor.viewport = window.viewport
    if created:
        app.exec()
