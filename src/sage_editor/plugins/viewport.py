"""Qt editor window with dockable viewport, object list and console."""

from __future__ import annotations

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

from engine.renderers.opengl.core import OpenGLRenderer
from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.entities.game_object import GameObject

from sage_editor.qt import GLWidget


class EditorWindow(QMainWindow):
    """Main editor window with dockable widgets."""

    def __init__(self, menus=None, toolbar=None) -> None:
        super().__init__()
        self.setWindowTitle("SAGE Editor")

        self.setDockNestingEnabled(True)

        self._engine = None
        self._game_window = None
        self.viewport = GLWidget(self)
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
        self.scene = Scene()
        self.scene.add_object(self.camera)
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
        self.objects.customContextMenuRequested.connect(self._context_menu)

    def set_renderer(self, renderer):
        self.viewport.renderer = renderer

    def set_objects(self, names):
        self.objects.clear()
        self.objects.addItems(list(names))

    def draw_scene(self) -> None:
        """Render the current scene to the viewport."""
        self.renderer.draw_scene(self.scene, self.camera)

    def start_game(self):
        from engine.core.engine import Engine
        from engine.game_window import GameWindow
        self._engine = Engine()
        self._game_window = GameWindow(self._engine)
        self._game_window.show()

    def _context_menu(self, point):
        menu = QMenu(self.objects)
        action = menu.addAction("Create Object")

        def create_object() -> None:
            count = self.objects.count() + 1
            obj = GameObject(name=f"Object {count}")
            self.scene.add_object(obj)
            self.objects.addItem(obj.name)
            self.draw_scene()

        action.triggered.connect(create_object)
        menu.exec(self.objects.mapToGlobal(point))


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
