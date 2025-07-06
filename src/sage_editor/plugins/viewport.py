"""Qt editor window with dockable viewport, object list and console.

The widget preview uses :class:`~engine.renderers.opengl.core.OpenGLRenderer` so
objects are drawn with hardware acceleration by default.
"""

from __future__ import annotations

import logging
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

    def __init__(self, window: "EditorWindow", *a, **k) -> None:
        super().__init__(window, *a, **k)
        self._window = window
        self._last_pos = None
        if hasattr(self, "setContextMenuPolicy"):
            self.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        if hasattr(self, "customContextMenuRequested"):
            self.customContextMenuRequested.connect(self._context_menu)

    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            self._last_pos = ev.position() if hasattr(ev, "position") else ev.pos()
            try:
                from PyQt6.QtGui import QCursor
                self.setCursor(QCursor(Qt.CursorShape.ClosedHandCursor))
            except Exception:
                log.exception("Failed to set cursor")

    def mouseMoveEvent(self, ev):  # pragma: no cover - gui interaction
        if self._last_pos is not None and ev.buttons() & Qt.MouseButton.LeftButton:
            pos = ev.position() if hasattr(ev, "position") else ev.pos()
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
        action = menu.addAction("Create Object")
        wx, wy = self._window.screen_to_world(point)

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
        self.draw_scene()

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

    def draw_scene(self) -> None:
        """Render the current scene to the viewport."""
        self.renderer.draw_scene(self.scene, self.camera)

    def start_game(self):
        from engine.core.engine import Engine
        from engine.game_window import GameWindow
        self.close_game()
        self._engine = Engine()
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

    def _list_context_menu(self, point):
        menu = QMenu(self.objects)
        action = menu.addAction("Create Object")
        action.triggered.connect(self.create_object)
        menu.exec(self.objects.mapToGlobal(point))

    def _object_selected(self, current, _prev):
        self.renderer.clear_gizmos()
        if current is not None:
            name = current.text()
            obj = self.scene.find_object(name)
            self.selected_obj = obj
            if obj is not None:
                if isinstance(obj, Camera):
                    left, bottom, w, h = obj.view_rect()
                    points = [
                        (left, bottom),
                        (left + w, bottom),
                        (left + w, bottom + h),
                        (left, bottom + h),
                        (left, bottom),
                    ]
                    g = gizmos.polyline_gizmo(points, color=(1, 0, 0, 1), frames=None)
                else:
                    g = gizmos.square_gizmo(obj.x, obj.y, size=20, color=(1, 0, 0, 1), frames=None)
                self.renderer.add_gizmo(g)
        else:
            self.selected_obj = None
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
