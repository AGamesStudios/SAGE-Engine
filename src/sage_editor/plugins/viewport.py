"""Qt editor window with dockable viewport, object list and console.

The widget preview uses :class:`~engine.renderers.opengl.core.OpenGLRenderer` so
objects are drawn with hardware acceleration by default.
"""

from __future__ import annotations

import logging
import json
from PyQt6.QtWidgets import (
    QApplication,
    QMainWindow,
    QListWidget,
    QDockWidget,
    QMenu,
    QPlainTextEdit,
    QMenuBar,
    QToolBar,
    QWidget,
    QSizePolicy,
    QSplitter,
    QPushButton,
    QVBoxLayout,
)
from PyQt6.QtGui import QAction
from PyQt6.QtCore import Qt
from engine.utils import units

from engine.renderers.opengl.core import OpenGLRenderer
from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from engine import gizmos

from sage_editor.qt import GLWidget

log = logging.getLogger(__name__)


class ViewportWidget(GLWidget):
    """GL widget with basic panning and zoom controls."""

    DRAG_THRESHOLD = 5

    def __init__(self, window: "EditorWindow", *a, **k) -> None:
        super().__init__(window, *a, **k)
        self._window = window
        self._last_pos = None
        self._press_pos = None
        self._dragging = False
        if hasattr(self, "setContextMenuPolicy"):
            self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        if hasattr(self, "customContextMenuRequested"):
            self.customContextMenuRequested.connect(self._context_menu)

    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            self._last_pos = ev.position() if hasattr(ev, "position") else ev.pos()
            self._press_pos = self._last_pos
            self._dragging = False
            try:
                from PyQt6.QtGui import QCursor
                self.setCursor(QCursor(Qt.CursorShape.ClosedHandCursor))
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
                self._window.draw_scene()
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
                self.unsetCursor()
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
            self._window.draw_scene()

    def _context_menu(self, point):  # pragma: no cover - gui interaction
        menu = QMenu(self)
        wx, wy = self._window.screen_to_world(point)
        obj = self._window.find_object_at(wx, wy)
        if obj is not None:
            self._window.select_object(obj)
            copy_a = menu.addAction("Copy")
            paste_a = menu.addAction("Paste")
            del_a = menu.addAction("Delete")
            copy_a.triggered.connect(self._window.copy_selected)
            paste_a.triggered.connect(self._window.paste_object)
            del_a.triggered.connect(self._window.delete_selected)
        else:
            action = menu.addAction("Create Object")

            def cb():
                self._window.create_object(wx, wy)

            action.triggered.connect(cb)
        menu.exec(self.mapToGlobal(point))



class EditorWindow(QMainWindow):
    """Main editor window with dockable widgets."""

    def __init__(self, menus=None, toolbar=None) -> None:
        super().__init__()
        self.setWindowTitle("SAGE Editor")

        self.setDockNestingEnabled(True)

        self._engine = None
        self._game_window = None
        self.viewport = ViewportWidget(self)
        self.console = QPlainTextEdit(self)
        self.properties = QPlainTextEdit(self)
        self.prop_apply = QPushButton("Apply", self)
        prop_wrap = QWidget(self)
        prop_layout = QVBoxLayout(prop_wrap)
        prop_layout.setContentsMargins(0, 0, 0, 0)
        prop_layout.addWidget(self.properties)
        prop_layout.addWidget(self.prop_apply)
        self.resources = QListWidget()

        # minimal scene used for previewing objects
        w = self.viewport.width() or 640
        h = self.viewport.height() or 480
        self.renderer = OpenGLRenderer(
            width=w, height=h, widget=self.viewport, vsync=False, keep_aspect=False
        )
        self.camera = Camera(width=w, height=h, active=True)
        self.scene = Scene(with_defaults=False)
        # keep the viewport camera separate from scene objects
        self.renderer.show_grid = True
        self.set_renderer(self.renderer)
        self.selected_obj: GameObject | None = None
        self._clipboard: dict | None = None

        splitter = QSplitter(Qt.Orientation.Vertical, self)
        splitter.addWidget(self.viewport)
        splitter.addWidget(self.console)
        splitter.setStretchFactor(0, 3)
        splitter.setStretchFactor(1, 1)
        self.setCentralWidget(splitter)

        menubar = QMenuBar(self)
        self.setMenuBar(menubar)
        if menus:
            for title, cb in menus:
                action = QAction(title, self)
                action.triggered.connect(cb)
                menubar.addAction(action)

        tbar = QToolBar(self)
        self.addToolBar(tbar)
        left_spacer = QWidget(self)
        left_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(left_spacer)

        run_action = QAction("Run", self)
        run_action.triggered.connect(self.start_game)
        tbar.addAction(run_action)

        right_spacer = QWidget(self)
        right_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(right_spacer)
        if toolbar:
            for title, cb in toolbar:
                action = QAction(title, self)
                action.triggered.connect(cb)
                tbar.addAction(action)

        self.objects = QListWidget()
        obj_dock = QDockWidget("Objects", self)
        obj_dock.setObjectName("ObjectsDock")
        obj_dock.setWidget(self.objects)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)

        prop_dock = QDockWidget("Properties", self)
        prop_dock.setObjectName("PropertiesDock")
        prop_dock.setWidget(prop_wrap)
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

    def set_objects(self, names):
        self.objects.clear()
        self.objects.addItems(list(names))

    def update_object_list(self):
        names = [obj.name for obj in self.scene.objects]
        self.set_objects(names)

    def update_properties(self):
        if self.selected_obj is None:
            self.properties.setPlainText("")
            return
        from engine.core.objects import object_to_dict
        try:
            data = object_to_dict(self.selected_obj) or {}
            def _default(obj):
                if isinstance(obj, set):
                    return list(obj)
                return str(obj)
            self.properties.setPlainText(json.dumps(data, indent=2, default=_default))
        except Exception:
            log.exception("Failed to display properties")

    def apply_properties(self):
        if self.selected_obj is None:
            return
        text = self.properties.toPlainText()
        try:
            data = json.loads(text)
        except Exception:
            log.exception("Invalid property data")
            return
        for k, v in data.items():
            if not hasattr(self.selected_obj, k):
                continue
            try:
                cur = getattr(self.selected_obj, k)
                if isinstance(cur, set) and isinstance(v, list):
                    v = set(v)
                if isinstance(cur, float) and isinstance(v, (int, float)):
                    setattr(self.selected_obj, k, float(v))
                elif isinstance(cur, int) and isinstance(v, int):
                    setattr(self.selected_obj, k, v)
                elif isinstance(v, type(cur)):
                    setattr(self.selected_obj, k, v)
                else:
                    log.warning(
                        "Type mismatch for %s: expected %s got %s",
                        k,
                        type(cur).__name__,
                        type(v).__name__,
                    )
            except Exception:
                log.exception("Failed to set %s", k)
        self.draw_scene()

    def find_object_at(self, x: float, y: float) -> GameObject | None:
        for obj in reversed(self.scene.objects):
            if isinstance(obj, Camera):
                continue
            left, bottom, w, h = obj.rect()
            if left <= x <= left + w and bottom <= y <= bottom + h:
                return obj
        return None

    def select_object(self, obj: GameObject | None) -> None:
        if obj is None:
            self.objects.setCurrentItem(None)
        else:
            for i in range(self.objects.count()):
                item = self.objects.item(i)
                if item.text() == obj.name:
                    self.objects.setCurrentItem(item)
                    break

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

    def draw_scene(self) -> None:
        """Render the current scene and refresh selection gizmos."""
        self._update_selection_gizmo()
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
            renderer="opengl",
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

    def paste_object(self) -> GameObject | None:
        if not self._clipboard:
            return None
        from engine.core.objects import object_from_dict
        try:
            obj = object_from_dict(dict(self._clipboard))
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
            menu.addAction("Copy").triggered.connect(self.copy_selected)
            menu.addAction("Paste").triggered.connect(self.paste_object)
            menu.addAction("Delete").triggered.connect(self.delete_selected)
            menu.addSeparator()
        action = menu.addAction("Create Object")
        action.triggered.connect(self.create_object)
        menu.exec(self.objects.mapToGlobal(point))

    def _object_selected(self, current, _prev):
        if current is not None:
            name = current.text()
            self.selected_obj = self.scene.find_object(name)
        else:
            self.selected_obj = None
        self.update_properties()
        self.draw_scene()

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
        app = QApplication([])
        created = True

    window = EditorWindow(editor._menus, editor._toolbar)
    window.resize(800, 600)
    window.show()

    editor.window = window
    editor.viewport = window.viewport
    if created:
        app.exec()
