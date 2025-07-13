from __future__ import annotations

import logging
import os
import math
import sys
from typing import Optional, cast

from PyQt6.QtWidgets import (  # type: ignore[import-not-found]
    QMainWindow,
    QListWidget,
    QDockWidget,
    QMenu,
    QPlainTextEdit,
    QMenuBar,
    QToolBar,
    QSizePolicy,
    QPushButton,  # noqa: F401 - used in dialogs
    QVBoxLayout,
    QHBoxLayout,
    QScrollArea,
    QLabel,
    QComboBox,
    QWidget,
)
try:  # optional for tests
    from PyQt6.QtWidgets import QAbstractItemView  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback when class missing
    QAbstractItemView = QWidget  # type: ignore[assignment]
try:  # optional for tests
    from PyQt6.QtWidgets import QFrame  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback when QFrame missing
    QFrame = QWidget  # type: ignore[misc]
from PyQt6.QtGui import QAction, QKeySequence  # type: ignore[import-not-found]
try:  # optional QPainter for fancy dial
    from PyQt6.QtGui import (
        QPainter,
        QColor,
        QPainterPath,
        QPointF,
    )  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback for stubs
    QPainter = None  # type: ignore[assignment]
    QColor = None  # type: ignore[assignment]
    QPainterPath = None  # type: ignore[assignment]
    QPointF = None  # type: ignore[assignment]

try:  # support minimal test stubs
    from PyQt6.QtWidgets import QTextEdit  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback when QTextEdit missing
    QTextEdit = QPlainTextEdit
from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
from engine.utils import units

from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from copy import deepcopy
from engine import gizmos

from sage_editor.qt import GLWidget
from sage_editor.plugins.editor_widgets import (
    ConsoleHandler,
    UndoStack,
    TransformBar,
    ModelBar,
    NoWheelLineEdit,
    NoWheelSpinBox,  # noqa: F401 - re-exported for widgets
    ProgressWheel,  # noqa: F401 - re-exported for widgets
    SnapSettingsWidget,
    SnapPopup,
)
from sage_editor.plugins.viewport_base import _ViewportMixin
import sage_editor.i18n as i18n
from sage_editor.i18n import LANGUAGES, set_language, tr

log = logging.getLogger(__name__)

class EditorWindow(QMainWindow):
    """Main editor window with dockable widgets."""

    def _create_viewport_widget(self, backend: str):
        from sage_editor import widgets as _w
        global ViewportWidget, SDLViewportWidget, PropertiesWidget
        ViewportWidget = _w.ViewportWidget
        SDLViewportWidget = _w.SDLViewportWidget
        PropertiesWidget = _w.PropertiesWidget
        if backend == "sdl":
            view = SDLViewportWidget(self)
        else:
            view = ViewportWidget(self)
        if hasattr(view, "setFocusPolicy"):
            view.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        container = QWidget(self)
        layout = QHBoxLayout(container)
        if hasattr(layout, "setContentsMargins"):
            # Small offset so the bar doesn't stick to the window frame
            layout.setContentsMargins(8, 8, 0, 0)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(4)
        bar = TransformBar(container)
        mbar = ModelBar(container)
        if hasattr(bar, "setSizePolicy"):
            pol = getattr(QSizePolicy, "Policy", QSizePolicy)
            horiz = getattr(pol, "Preferred", 0)
            vert = getattr(pol, "Expanding", 0)
            bar.setSizePolicy(horiz, vert)
        if hasattr(bar, "setFixedWidth"):
            bar.setFixedWidth(60)
        if hasattr(mbar, "setFixedWidth"):
            mbar.setFixedWidth(80)
        mbar.hide()
        layout.addWidget(bar)
        layout.addWidget(mbar)

        view_area = QWidget(container)
        vlayout = QVBoxLayout(view_area)
        if hasattr(vlayout, "setContentsMargins"):
            vlayout.setContentsMargins(0, 0, 0, 0)
        if hasattr(vlayout, "setSpacing"):
            vlayout.setSpacing(0)
        h_ruler = _w.RulerWidget(getattr(Qt.Orientation, "Horizontal", 1), view_area)
        vlayout.addWidget(h_ruler)

        row = QWidget(view_area)
        hlayout = QHBoxLayout(row)
        if hasattr(hlayout, "setContentsMargins"):
            hlayout.setContentsMargins(0, 0, 0, 0)
        if hasattr(hlayout, "setSpacing"):
            hlayout.setSpacing(0)
        v_ruler = _w.RulerWidget(getattr(Qt.Orientation, "Vertical", 2), row)
        hlayout.addWidget(v_ruler)
        hlayout.addWidget(view)
        vlayout.addWidget(row)

        layout.addWidget(view_area)
        frame = QFrame(view)
        frame.setObjectName("CameraPreviewFrame") if hasattr(frame, "setObjectName") else None
        if hasattr(frame, "setFrameShape"):
            frame.setFrameShape(getattr(QFrame, "Shape", QFrame).StyledPanel)
        fl = QVBoxLayout(frame)
        if hasattr(fl, "setContentsMargins"):
            fl.setContentsMargins(0, 0, 0, 0)
        preview = GLWidget(frame)
        if hasattr(preview, "setObjectName"):
            preview.setObjectName("CameraPreview")
        if hasattr(preview, "setFixedSize"):
            preview.setFixedSize(160, 120)
        fl.addWidget(preview)
        if hasattr(frame, "hide"):
            frame.hide()
        container.preview_widget = preview  # type: ignore[attr-defined]
        container.preview_frame = frame  # type: ignore[attr-defined]
        label = QLabel("X: 0  Y: 0", view)
        if hasattr(label, "setObjectName"):
            label.setObjectName("CursorLabel")
        if hasattr(label, "setStyleSheet"):
            label.setStyleSheet("color:#ddd;background:rgba(0,0,0,0.5);padding:2px;")
        if hasattr(label, "move"):
            label.move(8, 8)
        container.cursor_label = label  # type: ignore[attr-defined]
        if hasattr(view, "setMouseTracking"):
            view.setMouseTracking(True)
        container.viewport = view  # type: ignore[attr-defined]
        container.mode_bar = bar  # type: ignore[attr-defined]
        container.model_bar = mbar  # type: ignore[attr-defined]
        container.h_ruler = h_ruler  # type: ignore[attr-defined]
        container.v_ruler = v_ruler  # type: ignore[attr-defined]
        return container

    def __init__(self, menus=None, toolbar=None, *, backend: str = "opengl") -> None:
        super().__init__()
        self.setWindowTitle("SAGE Editor")

        self.setDockNestingEnabled(True)

        self._engine = None
        self._game_window = None
        container = self._create_viewport_widget(backend)
        self.viewport_container = container
        self.viewport = container.viewport  # type: ignore[attr-defined]
        self.mode_bar = container.mode_bar  # type: ignore[attr-defined]
        self.model_bar = container.model_bar  # type: ignore[attr-defined]
        self.cursor_label = container.cursor_label  # type: ignore[attr-defined]
        self.preview_widget = container.preview_widget  # type: ignore[attr-defined]
        self.preview_frame = container.preview_frame  # type: ignore[attr-defined]
        self.preview_renderer = None
        self.preview_camera = None
        self.cursor_pos: tuple[float, float] | None = None
        self.mode_bar.move_btn.clicked.connect(lambda: self.set_mode("move"))
        self.mode_bar.rotate_btn.clicked.connect(lambda: self.set_mode("rotate"))
        self.mode_bar.scale_btn.clicked.connect(lambda: self.set_mode("scale"))
        self.mode_bar.rect_btn.clicked.connect(lambda: self.set_mode("rect"))
        self.mode_bar.local_btn.clicked.connect(
            lambda checked: self.toggle_local(checked)
        )
        self.model_bar.vert_btn.clicked.connect(
            lambda: self.set_selection_mode("vertex")
        )
        self.model_bar.edge_btn.clicked.connect(
            lambda: self.set_selection_mode("edge")
        )
        self.model_bar.face_btn.clicked.connect(
            lambda: self.set_selection_mode("face")
        )
        self.model_bar.extrude_btn.clicked.connect(self.extrude_selection)
        self.model_bar.loop_btn.clicked.connect(self.loop_cut)
        self.model_bar.fill_btn.clicked.connect(self.toggle_fill)
        if hasattr(self.mode_bar.move_btn, "setChecked"):
            self.mode_bar.move_btn.setChecked(True)
        if hasattr(self.model_bar.vert_btn, "setChecked"):
            self.model_bar.vert_btn.setChecked(True)
        self.console = QTextEdit(self)
        self.console.setReadOnly(True)
        clear_a = QAction("Clear", self.console)
        if hasattr(clear_a, "triggered"):
            clear_a.triggered.connect(self.console.clear)
        copy_a = QAction("Copy All", self.console)

        def copy_all() -> None:
            self.console.selectAll()
            self.console.copy()
            cursor = self.console.textCursor()
            cursor.clearSelection()
            self.console.setTextCursor(cursor)

        if hasattr(copy_a, "triggered"):
            copy_a.triggered.connect(copy_all)
        if hasattr(self.console, "addAction"):
            self.console.addAction(clear_a)
            self.console.addAction(copy_a)
        self.console.setContextMenuPolicy(Qt.ContextMenuPolicy.ActionsContextMenu)
        ascii_html = (
            "<span style='color:#ff5555'>  _____         _____ ______   ______             _            </span><br>"
            "<span style='color:#ff8855'> / ____|  /\\   / ____|  ____| |  ____|           (_)           </span><br>"
            "<span style='color:#ffff55'>| (___   /  \\ | |  __| |__    | |__   _ __   __ _ _ _ __   ___ </span><br>"
            "<span style='color:#55ff55'> \\___ \\ / /\\ \\| | |_ |  __|   |  __| | '_ \\ / _` | | '_ \\ / _ \\</span><br>"
            "<span style='color:#55ffff'> ____) / ____ \\ |__| | |____  | |____| | | | (_| | | | | |  __/</span><br>"
            "<span style='color:#5555ff'>|_____/_/    \\_\\_____|______| |______|_| |_|\\__, |_|_| |_|\\___|</span><br>"
            "<span style='color:#8855ff'>                                             __/ |             </span><br>"
            "<span style='color:#ff55ff'>                                            |___/              </span>"
        )
        ascii_plain = (
            "  _____         _____ ______   ______             _\n"
            " / ____|  /\\   / ____|  ____| |  ____|           (_)\n"
            "| (___   /  \\ | |  __| |__    | |__   _ __   __ _ _ _ __   ___\n"
            " \\___ \\ / /\\ \\| | |_ |  __|   |  __| | '_ \\ / _` | | '_ \\ / _ \\n"
            " ____) / ____ \\ |__| | |____  | |____| | | | (_| | | | | |  __/\n"
            "|_____/_/    \\_\\_____|______| |______|_| |_|\\__, |_|_| |_|\\___|\n"
            "                                             __/ |\n"
            "                                            |___/\n"
        )
        if hasattr(self.console, "appendHtml"):
            self.console.appendHtml(ascii_html)
            self.console.append("Welcome to SAGE Editor")
        else:
            self.console.setPlainText(ascii_plain + "\nWelcome to SAGE Editor")
        from engine.utils.log import logger

        self._console_handler = ConsoleHandler(self.console)
        logger.addHandler(self._console_handler)
        self.properties = PropertiesWidget(self)
        self.properties.set_object(None)
        for edit in [
            self.properties.name_edit,
            self.properties.tags_edit,
            self.properties.pos_x,
            self.properties.pos_y,
            self.properties.scale_x,
            self.properties.scale_y,
            self.properties.pivot_x,
            self.properties.pivot_y,
            self.properties.image_edit,
        ]:
            if hasattr(edit, "editingFinished"):
                edit.editingFinished.connect(self.apply_properties)
            if hasattr(edit, "valueChanged"):
                edit.valueChanged.connect(lambda *_: self.apply_properties(False))
        for box in [
            self.properties.visible_check,
            self.properties.flip_x,
            self.properties.flip_y,
            self.properties.smooth_check,
            getattr(self.properties, "physics_enabled", None),
        ]:
            if box is not None and hasattr(box, "stateChanged"):
                box.stateChanged.connect(lambda *_: self.apply_properties(False))
        if hasattr(self.properties.rot_dial, "valueChanged"):
            self.properties.rot_dial.valueChanged.connect(lambda *_: self.apply_properties(False))
        if hasattr(self.properties.role_combo, "currentIndexChanged"):
            self.properties.role_combo.currentIndexChanged.connect(lambda *_: self.apply_properties(False))
        if hasattr(self.properties.shape_combo, "currentIndexChanged"):
            self.properties.shape_combo.currentIndexChanged.connect(lambda *_: self.apply_properties(False))
        body = getattr(self.properties, "body_combo", None)
        if body is not None and hasattr(body, "currentIndexChanged"):
            body.currentIndexChanged.connect(lambda *_: self.apply_properties(False))
        self.resources = QListWidget()
        self.resource_root = ""
        self.resources_label = QLabel("Resources:")
        res_container = QWidget(self)
        res_layout = QVBoxLayout(res_container)
        res_layout.setContentsMargins(0, 0, 0, 0)
        res_layout.addWidget(self.resources_label)
        res_layout.addWidget(self.resources)

        # minimal scene used for previewing objects
        w = self.viewport.width() or 640
        h = self.viewport.height() or 480
        try:
            import viewport as _vp  # type: ignore
        except Exception:
            from sage_editor.plugins import viewport as _vp
        vp_mod = sys.modules.get("viewport", _vp)
        if backend == "opengl":
            rcls = vp_mod.OpenGLRenderer
        else:
            from engine.renderers import get_renderer
            rcls = get_renderer(backend)
            if rcls is None:
                self.log_warning(f"Renderer '{backend}' unavailable; falling back to OpenGL")
                rcls = vp_mod.OpenGLRenderer
        self.renderer_backend = backend if rcls is not vp_mod.OpenGLRenderer else "opengl"
        self.renderer = rcls(
            width=w,
            height=h,
            widget=self.viewport,
            vsync=False,
            keep_aspect=False,
        )
        self.camera = Camera(width=w, height=h, active=True)
        self.scene = Scene(with_defaults=False)
        self.undo_stack = UndoStack()
        self.project_path: str | None = None
        # keep the viewport camera separate from scene objects
        self.renderer.show_grid = True
        self.mirror_resize = False
        self.local_coords = False
        self.rulers_visible = True
        self.cursor_visible = True
        self.snap_to_grid = False
        self.move_step = 1.0
        self.rotate_step = 15.0
        self.scale_step = 0.1
        self.modeling = False
        self.transform_mode = "move"
        self.selection_mode = "vertex"
        self.selected_vertices: set[int] = set()
        self.selected_edges: set[int] = set()
        self.selected_face = False
        self.set_renderer(self.renderer)
        self.selected_obj: Optional[GameObject] = None
        self.selected_objs: list[GameObject] = []
        self._clipboard: dict | None = None

        self.setCentralWidget(self.viewport_container)
        console_dock = QDockWidget("Console", self)
        console_dock.setObjectName("ConsoleDock")
        console_dock.setWidget(self.console)
        if hasattr(console_dock, "setMinimumHeight"):
            console_dock.setMinimumHeight(120)
        area = getattr(Qt.DockWidgetArea, "BottomDockWidgetArea", Qt.DockWidgetArea.LeftDockWidgetArea)
        self.addDockWidget(area, console_dock)
        self.console_dock = console_dock  # type: ignore[assignment]

        menubar = QMenuBar(self)
        self.setMenuBar(menubar)
        file_menu = menubar.addMenu(tr("File")) if hasattr(menubar, "addMenu") else None
        self.file_menu = file_menu
        open_p = file_menu.addAction("Open Project") if file_menu and hasattr(file_menu, "addAction") else None
        save_p = file_menu.addAction("Save Project") if file_menu and hasattr(file_menu, "addAction") else None
        shot_p = file_menu.addAction("Screenshot...") if file_menu and hasattr(file_menu, "addAction") else None
        edit_menu = menubar.addMenu(tr("Edit")) if hasattr(menubar, "addMenu") else None
        self.edit_menu = edit_menu
        undo_m = edit_menu.addAction("Undo") if edit_menu and hasattr(edit_menu, "addAction") else None
        redo_m = edit_menu.addAction("Redo") if edit_menu and hasattr(edit_menu, "addAction") else None
        copy_m = edit_menu.addAction("Copy") if edit_menu and hasattr(edit_menu, "addAction") else None
        paste_m = edit_menu.addAction("Paste") if edit_menu and hasattr(edit_menu, "addAction") else None
        del_m = edit_menu.addAction("Delete") if edit_menu and hasattr(edit_menu, "addAction") else None
        self.copy_action = copy_m
        self.paste_action = paste_m
        self.delete_action = del_m
        self.undo_action = undo_m
        self.redo_action = redo_m
        if copy_m is not None and hasattr(copy_m, "setShortcut"):
            copy_m.setShortcut(QKeySequence("Ctrl+C"))
        if paste_m is not None and hasattr(paste_m, "setShortcut"):
            paste_m.setShortcut(QKeySequence("Ctrl+V"))
        if del_m is not None and hasattr(del_m, "setShortcut"):
            del_m.setShortcut(QKeySequence("Delete"))
        if undo_m is not None and hasattr(undo_m, "setShortcut"):
            undo_m.setShortcut(QKeySequence("Ctrl+Z"))
        if redo_m is not None and hasattr(redo_m, "setShortcut"):
            redo_m.setShortcut(QKeySequence("Ctrl+Y"))
        if undo_m is not None and hasattr(undo_m, "triggered"):
            undo_m.triggered.connect(self.undo)
        if redo_m is not None and hasattr(redo_m, "triggered"):
            redo_m.triggered.connect(self.redo)
        engine_menu = menubar.addMenu(tr("Engine")) if hasattr(menubar, "addMenu") else None
        self.engine_menu = engine_menu
        renderer_menu = engine_menu.addMenu("Renderer") if engine_menu and hasattr(engine_menu, "addMenu") else None
        if open_p is not None and hasattr(open_p, "triggered"):
            open_p.triggered.connect(self.open_project_dialog)
        if save_p is not None and hasattr(save_p, "triggered"):
            save_p.triggered.connect(self.save_project_dialog)
        if shot_p is not None and hasattr(shot_p, "triggered"):
            shot_p.triggered.connect(self.open_screenshot_dialog)
        ogl_action = renderer_menu.addAction("OpenGL") if renderer_menu and hasattr(renderer_menu, "addAction") else None
        sdl_action = renderer_menu.addAction("SDL") if renderer_menu and hasattr(renderer_menu, "addAction") else None
        if ogl_action is not None and hasattr(ogl_action, "triggered"):
            ogl_action.triggered.connect(lambda: self.change_renderer("opengl"))
        if sdl_action is not None and hasattr(sdl_action, "triggered"):
            sdl_action.triggered.connect(lambda: self.change_renderer("sdl"))
        view_menu = engine_menu.addMenu(tr("View")) if engine_menu and hasattr(engine_menu, "addMenu") else None
        self.view_menu = view_menu
        grid_act = view_menu.addAction("Show Grid") if view_menu and hasattr(view_menu, "addAction") else None
        if grid_act is not None:
            if hasattr(grid_act, "setCheckable"):
                grid_act.setCheckable(True)
            if hasattr(grid_act, "setChecked"):
                grid_act.setChecked(True)
            if hasattr(grid_act, "triggered"):
                grid_act.triggered.connect(self.toggle_grid)

        snap_act = view_menu.addAction("Snap to Grid") if view_menu and hasattr(view_menu, "addAction") else None
        if snap_act is not None:
            if hasattr(snap_act, "setCheckable"):
                snap_act.setCheckable(True)
            if hasattr(snap_act, "setChecked"):
                snap_act.setChecked(False)
            if hasattr(snap_act, "triggered"):
                snap_act.triggered.connect(self.toggle_snap)

        snap_settings = view_menu.addAction("Snap Settings...") if view_menu and hasattr(view_menu, "addAction") else None
        if snap_settings is not None and hasattr(snap_settings, "triggered"):
            snap_settings.triggered.connect(self.open_snap_dialog)

        axes_act = view_menu.addAction("Show Axes") if view_menu and hasattr(view_menu, "addAction") else None
        if axes_act is not None:
            if hasattr(axes_act, "setCheckable"):
                axes_act.setCheckable(True)
            if hasattr(axes_act, "setChecked"):
                axes_act.setChecked(True)
            if hasattr(axes_act, "triggered"):
                axes_act.triggered.connect(self.toggle_axes)

        mirror_act = view_menu.addAction("Mirror Resize") if view_menu and hasattr(view_menu, "addAction") else None
        if mirror_act is not None:
            if hasattr(mirror_act, "setCheckable"):
                mirror_act.setCheckable(True)
            if hasattr(mirror_act, "setChecked"):
                mirror_act.setChecked(False)
            if hasattr(mirror_act, "triggered"):
                mirror_act.triggered.connect(self.toggle_mirror)

        local_act = view_menu.addAction("Local Coordinates") if view_menu and hasattr(view_menu, "addAction") else None
        if local_act is not None:
            if hasattr(local_act, "setCheckable"):
                local_act.setCheckable(True)
            if hasattr(local_act, "setChecked"):
                local_act.setChecked(False)
            if hasattr(local_act, "triggered"):
                local_act.triggered.connect(self.toggle_local)

        self.settings_menu = menubar.addMenu(tr("Settings")) if hasattr(menubar, "addMenu") else None
        lang_menu = self.settings_menu.addMenu(tr("Language")) if self.settings_menu and hasattr(self.settings_menu, "addMenu") else None
        self._lang_menu = lang_menu
        self._lang_actions = {}
        if lang_menu:
            for name in LANGUAGES:
                act = lang_menu.addAction(tr(name))
                if hasattr(act, "setCheckable"):
                    act.setCheckable(True)
                if name == i18n.CURRENT_LANGUAGE and hasattr(act, "setChecked"):
                    act.setChecked(True)
                if hasattr(act, "triggered"):
                    act.triggered.connect(lambda _c, n=name: self.change_language(n))
                self._lang_actions[name] = act

        self.about_menu = menubar.addMenu(tr("About")) if hasattr(menubar, "addMenu") else None
        self.about_action = self.about_menu.addAction(tr("About")) if self.about_menu and hasattr(self.about_menu, "addAction") else None
        if self.about_action is not None and hasattr(self.about_action, "triggered"):
            self.about_action.triggered.connect(self.show_about_dialog)

        if copy_m is not None and hasattr(copy_m, "triggered"):
            copy_m.triggered.connect(self.copy_selected)
        if paste_m is not None and hasattr(paste_m, "triggered"):
            paste_m.triggered.connect(self.paste_object)
        if del_m is not None and hasattr(del_m, "triggered"):
            del_m.triggered.connect(self.delete_selected)

        if menus:
            for title, cb in menus:
                action = QAction(title, self)
                if action is not None and hasattr(action, "triggered"):
                    action.triggered.connect(cb)
                if hasattr(menubar, "addAction"):
                    menubar.addAction(action)

        tbar = QToolBar(self)
        if hasattr(tbar, "setMovable"):
            tbar.setMovable(False)
        try:
            from PyQt6.QtCore import QSize  # type: ignore
        except Exception:
            pass
        else:
            tbar.setIconSize(QSize(20, 20))
        self.addToolBar(tbar)
        left_spacer = QWidget(self)
        left_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(left_spacer)

        self.mode_combo = QComboBox(self)
        self.mode_combo.addItems(["Edit", "Model"])
        self.mode_combo.setCurrentIndex(0)
        self.mode_combo.currentIndexChanged.connect(
            lambda i: self.toggle_model(bool(i))
        )
        tbar.addWidget(self.mode_combo)

        run_action = QAction("Run", self)
        if run_action is not None and hasattr(run_action, "triggered"):
            run_action.triggered.connect(self.start_game)
        if hasattr(tbar, "addAction"):
            tbar.addAction(run_action)

        shot_action = QAction("Screenshot", self)
        if shot_action is not None:
            if hasattr(shot_action, "triggered"):
                shot_action.triggered.connect(self.open_screenshot_dialog)
            if hasattr(tbar, "addAction"):
                tbar.addAction(shot_action)

        right_spacer = QWidget(self)
        right_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(right_spacer)
        if toolbar:
            for title, cb in toolbar:
                action = QAction(title, self)
                if action is not None and hasattr(action, "triggered"):
                    action.triggered.connect(cb)
                if hasattr(tbar, "addAction"):
                    tbar.addAction(action)

        quickbar = QToolBar(self)
        if hasattr(quickbar, "setMovable"):
            quickbar.setMovable(False)
        self.addToolBar(quickbar)
        self.quickbar = quickbar  # type: ignore[attr-defined]
        self.grid_action = QAction("Grid", self)
        if hasattr(self.grid_action, "setCheckable"):
            self.grid_action.setCheckable(True)
        if hasattr(self.grid_action, "setChecked"):
            self.grid_action.setChecked(True)
        if hasattr(self.grid_action, "toggled"):
            self.grid_action.toggled.connect(self.toggle_grid)
        quickbar.addAction(self.grid_action)

        try:  # pragma: no cover - Qt only
            from PyQt6.QtWidgets import QToolButton  # type: ignore[import-not-found]
        except Exception:  # pragma: no cover - fallback for tests
            QToolButton = QWidget  # type: ignore[assignment]

        self.snap_button = QToolButton()
        if hasattr(self.snap_button, "setText"):
            self.snap_button.setText("Snap")
        if hasattr(self.snap_button, "setCheckable"):
            self.snap_button.setCheckable(True)
        if hasattr(self.snap_button, "setChecked"):
            self.snap_button.setChecked(False)
        if hasattr(self.snap_button, "toggled"):
            self.snap_button.toggled.connect(self.toggle_snap)
        quickbar.addWidget(self.snap_button)

        self.snap_menu_btn = QToolButton()
        if hasattr(self.snap_menu_btn, "setText"):
            self.snap_menu_btn.setText("▼")
        if hasattr(self.snap_menu_btn, "setAutoRaise"):
            self.snap_menu_btn.setAutoRaise(True)
        if hasattr(self.snap_menu_btn, "clicked"):
            self.snap_menu_btn.clicked.connect(
                lambda: self.snap_popup.show_near(self.snap_menu_btn)
            )
        if hasattr(self.snap_menu_btn, "setVisible"):
            self.snap_menu_btn.setVisible(False)
        quickbar.addWidget(self.snap_menu_btn)

        if hasattr(quickbar, "addSeparator"):
            quickbar.addSeparator()

        self.axes_action = QAction("Axes", self)
        if hasattr(self.axes_action, "setCheckable"):
            self.axes_action.setCheckable(True)
        if hasattr(self.axes_action, "setChecked"):
            self.axes_action.setChecked(getattr(self.renderer, "show_axes", True))
        if hasattr(self.axes_action, "toggled"):
            self.axes_action.toggled.connect(self.toggle_axes)
        quickbar.addAction(self.axes_action)

        self.ruler_action = QAction("Rulers", self)
        if hasattr(self.ruler_action, "setCheckable"):
            self.ruler_action.setCheckable(True)
        if hasattr(self.ruler_action, "setChecked"):
            self.ruler_action.setChecked(True)
        if hasattr(self.ruler_action, "toggled"):
            self.ruler_action.toggled.connect(self.toggle_rulers)
        quickbar.addAction(self.ruler_action)

        self.cursor_action = QAction("Cursor", self)
        if hasattr(self.cursor_action, "setCheckable"):
            self.cursor_action.setCheckable(True)
        if hasattr(self.cursor_action, "setChecked"):
            self.cursor_action.setChecked(True)
        if hasattr(self.cursor_action, "toggled"):
            self.cursor_action.toggled.connect(self.toggle_cursor_label)
        quickbar.addAction(self.cursor_action)

        self.local_action = QAction("Local", self)
        if hasattr(self.local_action, "setCheckable"):
            self.local_action.setCheckable(True)
        if hasattr(self.local_action, "setChecked"):
            self.local_action.setChecked(False)
        if hasattr(self.local_action, "toggled"):
            self.local_action.toggled.connect(self.toggle_local)
        quickbar.addAction(self.local_action)

        if hasattr(quickbar, "addSeparator"):
            quickbar.addSeparator()

        self.objects = QListWidget()
        if hasattr(self.objects, "setSelectionMode"):
            self.objects.setSelectionMode(
                QAbstractItemView.SelectionMode.ExtendedSelection
            )
        obj_dock = QDockWidget("Objects", self)
        obj_dock.setObjectName("ObjectsDock")
        obj_dock.setWidget(self.objects)
        if hasattr(obj_dock, "setMinimumWidth"):
            obj_dock.setMinimumWidth(150)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)

        prop_dock = QDockWidget("Properties", self)
        prop_dock.setObjectName("PropertiesDock")
        prop_scroll = QScrollArea(self)
        prop_scroll.setWidgetResizable(True)
        prop_scroll.setWidget(self.properties)
        prop_dock.setWidget(prop_scroll)
        if hasattr(prop_dock, "setMinimumWidth"):
            prop_dock.setMinimumWidth(220)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, prop_dock)
        self.splitDockWidget(obj_dock, prop_dock, Qt.Orientation.Vertical)

        res_dock = QDockWidget("Resources", self)
        res_dock.setObjectName("ResourcesDock")
        res_dock.setWidget(res_container)
        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, res_dock)

        snap_widget = SnapSettingsWidget(self)
        snap_dock = QDockWidget("Snap", self)
        snap_dock.setObjectName("SnapDock")
        snap_dock.setWidget(snap_widget)
        if hasattr(snap_dock, "hide"):
            snap_dock.hide()
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, snap_dock)
        self.snap_dock = snap_dock  # type: ignore[assignment]
        self.snap_popup = SnapPopup(self)
        if hasattr(self.snap_popup, "hide"):
            self.snap_popup.hide()

        self.apply_translation()

        self.objects.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.objects.customContextMenuRequested.connect(self._list_context_menu)
        if hasattr(self.objects, "itemSelectionChanged"):
            self.objects.itemSelectionChanged.connect(self._object_selected)
        else:
            self.objects.currentItemChanged.connect(self._object_selected)

        self.selected_obj = None
        self.update_object_list()
        self._reposition_preview()
        self.draw_scene()
        self._update_rulers()

    def log_warning(self, text: str) -> None:
        """Display *text* in the console dock."""
        self.console.append(text)

    def toggle_grid(self, checked: bool) -> None:
        if hasattr(self.renderer, "show_grid"):
            self.renderer.show_grid = checked
            self.draw_scene(update_list=False)

    def toggle_snap(self, checked: bool) -> None:
        self.snap_to_grid = bool(checked)
        pop = getattr(self, "snap_popup", None)
        menu_btn = getattr(self, "snap_menu_btn", None)
        if menu_btn is not None and hasattr(menu_btn, "setVisible"):
            menu_btn.setVisible(checked)
        if not checked and pop is not None:
            pop.hide()

    def toggle_mirror(self, checked: bool) -> None:
        self.mirror_resize = bool(checked)

    def toggle_local(self, checked: bool) -> None:
        """Toggle local coordinate mode."""
        self.local_coords = bool(checked)
        if hasattr(self.mode_bar, "local_btn") and hasattr(
            self.mode_bar.local_btn, "setChecked"
        ):
            self.mode_bar.local_btn.setChecked(self.local_coords)
        self.draw_scene(update_list=False)

    def toggle_rulers(self, checked: bool) -> None:
        """Show or hide the viewport rulers."""
        self.rulers_visible = bool(checked)
        cont = self.viewport_container
        h_ruler = getattr(cont, "h_ruler", None)
        v_ruler = getattr(cont, "v_ruler", None)
        if h_ruler is not None and hasattr(h_ruler, "setVisible"):
            h_ruler.setVisible(checked)
        if v_ruler is not None and hasattr(v_ruler, "setVisible"):
            v_ruler.setVisible(checked)

    def toggle_cursor_label(self, checked: bool) -> None:
        """Show or hide the cursor coordinate label."""
        self.cursor_visible = bool(checked)
        if hasattr(self.cursor_label, "setVisible"):
            self.cursor_label.setVisible(checked)

    def toggle_axes(self, checked: bool) -> None:
        """Show or hide the world coordinate axes gizmo."""
        if hasattr(self.renderer, "show_axes"):
            self.renderer.show_axes = bool(checked)
            self.draw_scene(update_list=False)

    def change_language(self, name: str) -> None:
        """Switch UI text to the selected language."""
        set_language(name)
        for lang, act in getattr(self, "_lang_actions", {}).items():
            if hasattr(act, "setChecked"):
                act.setChecked(lang == name)
            if hasattr(act, "setText"):
                act.setText(tr(lang))
        self.apply_translation()

    def apply_translation(self) -> None:
        """Refresh menu titles using the current language."""
        if hasattr(self, "file_menu") and hasattr(self.file_menu, "setTitle"):
            self.file_menu.setTitle(tr("File"))
        if hasattr(self, "edit_menu") and hasattr(self.edit_menu, "setTitle"):
            self.edit_menu.setTitle(tr("Edit"))
        if hasattr(self, "engine_menu") and hasattr(self.engine_menu, "setTitle"):
            self.engine_menu.setTitle(tr("Engine"))
        if hasattr(self, "settings_menu") and hasattr(self.settings_menu, "setTitle"):
            self.settings_menu.setTitle(tr("Settings"))
        if hasattr(self, "view_menu") and hasattr(self.view_menu, "setTitle"):
            self.view_menu.setTitle(tr("View"))
        if hasattr(self, "about_menu") and hasattr(self.about_menu, "setTitle"):
            self.about_menu.setTitle(tr("About"))
        if hasattr(self, "about_action") and hasattr(self.about_action, "setText"):
            self.about_action.setText(tr("About"))
        if hasattr(self, "_lang_actions"):
            lang_menu = getattr(self, "_lang_menu", None)
            if lang_menu is not None and hasattr(lang_menu, "setTitle"):
                lang_menu.setTitle(tr("Language"))
            for lang, act in self._lang_actions.items():
                if hasattr(act, "setText"):
                    act.setText(tr(lang))

    def show_about_dialog(self) -> None:
        from PyQt6.QtWidgets import QMessageBox  # type: ignore[import-not-found]

        QMessageBox.about(self, "SAGE Engine", tr("About_Message"))

    def open_snap_dock(self) -> None:
        """Display the snap settings dock."""
        dock = getattr(self, "snap_dock", None)
        if dock is not None:
            widget = dock.widget()
            if widget is not None:
                if hasattr(widget, "move_spin"):
                    widget.move_spin.setValue(self.move_step)
                if hasattr(widget, "rot_spin"):
                    widget.rot_spin.setValue(self.rotate_step)
                if hasattr(widget, "scale_spin"):
                    widget.scale_spin.setValue(self.scale_step)
            if hasattr(dock, "show"):
                dock.show()
            if hasattr(dock, "raise_"):
                dock.raise_()
        if getattr(self, "snap_popup", None) is not None:
            self.snap_popup.hide()

    def toggle_model(self, modeling: bool) -> None:
        """Switch between edit and model modes."""
        self.modeling = modeling
        if hasattr(self, "mode_combo") and hasattr(self.mode_combo, "currentIndex"):
            new_idx = 1 if modeling else 0
            try:
                cur = self.mode_combo.currentIndex()
            except Exception:
                cur = new_idx
            if cur != new_idx and hasattr(self.mode_combo, "setCurrentIndex"):
                self.mode_combo.setCurrentIndex(new_idx)
        if hasattr(self.model_bar, "setVisible"):
            self.model_bar.setVisible(modeling)
        if hasattr(self.mode_bar, "setVisible"):
            self.mode_bar.setVisible(not modeling)
        if not modeling:
            self.selected_vertices.clear()
            self.selected_edges.clear()
            self.selected_face = False
        self.draw_scene(update_list=False)

    def update_cursor(self, x: float, y: float) -> None:
        """Store cursor world coordinates and update the overlay label."""
        self.cursor_pos = (x, y)
        if hasattr(self.cursor_label, "setText"):
            self.cursor_label.setText(f"X: {x:.1f}  Y: {y:.1f}")
            if hasattr(self.cursor_label, "adjustSize"):
                self.cursor_label.adjustSize()
        self._update_rulers()
        self.draw_scene(update_list=False)

    def set_mode(self, mode: str) -> None:
        self.transform_mode = mode
        if mode == "move":
            if hasattr(self.mode_bar.move_btn, "setChecked"):
                self.mode_bar.move_btn.setChecked(True)
            if hasattr(self.mode_bar.rotate_btn, "setChecked"):
                self.mode_bar.rotate_btn.setChecked(False)
            if hasattr(self.mode_bar.scale_btn, "setChecked"):
                self.mode_bar.scale_btn.setChecked(False)
            if hasattr(self.mode_bar.rect_btn, "setChecked"):
                self.mode_bar.rect_btn.setChecked(False)
        elif mode == "rotate":
            if hasattr(self.mode_bar.move_btn, "setChecked"):
                self.mode_bar.move_btn.setChecked(False)
            if hasattr(self.mode_bar.rotate_btn, "setChecked"):
                self.mode_bar.rotate_btn.setChecked(True)
            if hasattr(self.mode_bar.scale_btn, "setChecked"):
                self.mode_bar.scale_btn.setChecked(False)
            if hasattr(self.mode_bar.rect_btn, "setChecked"):
                self.mode_bar.rect_btn.setChecked(False)
        elif mode == "scale":
            if hasattr(self.mode_bar.move_btn, "setChecked"):
                self.mode_bar.move_btn.setChecked(False)
            if hasattr(self.mode_bar.rotate_btn, "setChecked"):
                self.mode_bar.rotate_btn.setChecked(False)
            if hasattr(self.mode_bar.scale_btn, "setChecked"):
                self.mode_bar.scale_btn.setChecked(True)
            if hasattr(self.mode_bar.rect_btn, "setChecked"):
                self.mode_bar.rect_btn.setChecked(False)
        else:
            if hasattr(self.mode_bar.move_btn, "setChecked"):
                self.mode_bar.move_btn.setChecked(False)
            if hasattr(self.mode_bar.rotate_btn, "setChecked"):
                self.mode_bar.rotate_btn.setChecked(False)
            if hasattr(self.mode_bar.scale_btn, "setChecked"):
                self.mode_bar.scale_btn.setChecked(False)
            if hasattr(self.mode_bar.rect_btn, "setChecked"):
                self.mode_bar.rect_btn.setChecked(True)
        self.draw_scene(update_list=False)

    def keyPressEvent(self, ev):  # pragma: no cover - ui hotkeys
        key = ev.key() if hasattr(ev, "key") else None
        if self.modeling:
            if key == getattr(Qt.Key, "E", 0):
                self.extrude_selection()
                return
            if key == getattr(Qt.Key, "F", 0):
                self.new_face_from_edge()
                return
            if key == getattr(Qt.Key, "L", 0):
                self.loop_cut()
                return
        super().keyPressEvent(ev)

    # modeling helpers -------------------------------------------------
    def set_selection_mode(self, mode: str) -> None:
        self.selection_mode = mode
        self.selected_vertices.clear()
        self.selected_edges.clear()
        self.selected_face = False
        if mode == "vertex":
            if hasattr(self.model_bar.vert_btn, "setChecked"):
                self.model_bar.vert_btn.setChecked(True)
            if hasattr(self.model_bar.edge_btn, "setChecked"):
                self.model_bar.edge_btn.setChecked(False)
            if hasattr(self.model_bar.face_btn, "setChecked"):
                self.model_bar.face_btn.setChecked(False)
        elif mode == "edge":
            if hasattr(self.model_bar.vert_btn, "setChecked"):
                self.model_bar.vert_btn.setChecked(False)
            if hasattr(self.model_bar.edge_btn, "setChecked"):
                self.model_bar.edge_btn.setChecked(True)
            if hasattr(self.model_bar.face_btn, "setChecked"):
                self.model_bar.face_btn.setChecked(False)
        else:
            if hasattr(self.model_bar.vert_btn, "setChecked"):
                self.model_bar.vert_btn.setChecked(False)
            if hasattr(self.model_bar.edge_btn, "setChecked"):
                self.model_bar.edge_btn.setChecked(False)
            if hasattr(self.model_bar.face_btn, "setChecked"):
                self.model_bar.face_btn.setChecked(True)

    def extrude_selection(self) -> None:
        """Duplicate selected vertices and prepare them for dragging."""
        if self.selection_mode != "vertex":
            return
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        mesh = obj.mesh
        new_selection: set[int] = set()
        for idx in sorted(self.selected_vertices, reverse=True):
            if 0 <= idx < len(mesh.vertices):
                vx, vy = mesh.vertices[idx]
                mesh.vertices.insert(idx + 1, (vx, vy))
                self.selected_vertices = {
                    i + 1 if i > idx else i for i in self.selected_vertices
                }
                new_selection.add(idx + 1)
        if new_selection:
            self.selected_vertices = new_selection
        self.draw_scene(update_list=False)

    def new_face_from_edge(self) -> None:
        """Extrude the selected edge to form a quad."""
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        verts = obj.mesh.vertices
        if self.selected_edges:
            idx = next(iter(self.selected_edges))
        elif (
            len(self.selected_vertices) == 2
            and all(0 <= i < len(verts) for i in self.selected_vertices)
        ):
            a, b = sorted(self.selected_vertices)
            if b == a + 1 or (a == len(verts) - 1 and b == 0):
                idx = a
            else:
                return
        else:
            return
        nx, ny = self._edge_normal(verts, idx)
        off = 0.5
        a = verts[idx]
        b = verts[(idx + 1) % len(verts)]
        new_b = (b[0] + nx * off, b[1] + ny * off)
        new_a = (a[0] + nx * off, a[1] + ny * off)
        new_verts = verts[: idx + 1] + [new_b, new_a] + verts[idx + 1 :]
        from engine.mesh_utils import create_polygon_mesh
        poly = create_polygon_mesh(new_verts)
        obj.mesh.vertices = poly.vertices
        obj.mesh.indices = poly.indices
        self.selected_edges = {idx + 1}
        self.selected_vertices = {idx + 1, idx + 2}
        self.draw_scene(update_list=False)

    def loop_cut(self) -> None:
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        verts = obj.mesh.vertices
        new_verts: list[tuple[float, float]] = []
        for i, v in enumerate(verts):
            nxt = verts[(i + 1) % len(verts)]
            new_verts.append(v)
            mid = ((v[0] + nxt[0]) / 2, (v[1] + nxt[1]) / 2)
            new_verts.append(mid)
        from engine.mesh_utils import create_polygon_mesh
        poly = create_polygon_mesh(new_verts)
        obj.mesh.vertices = poly.vertices
        obj.mesh.indices = poly.indices
        self.draw_scene(update_list=False)

    def toggle_fill(self) -> None:
        obj = self.selected_obj
        if obj is None or not hasattr(obj, "filled"):
            return
        self.undo_stack.snapshot(self.scene)
        obj.filled = not obj.filled
        self.update_properties()
        self.draw_scene(update_list=False)

    def translate_selection(self, dx: float, dy: float) -> None:
        """Move the currently selected mesh elements by ``dx, dy`` world units."""
        obj = self.selected_obj
        if obj is None or getattr(obj, "mesh", None) is None:
            return
        self.undo_stack.snapshot(self.scene)
        verts = obj.mesh.vertices
        if self.selection_mode == "vertex":
            indices = set(self.selected_vertices)
        elif self.selection_mode == "edge":
            indices = set()
            for idx in self.selected_edges:
                indices.add(idx)
                indices.add((idx + 1) % len(verts))
        else:  # face
            if not self.selected_face:
                return
            indices = set(range(len(verts)))
        for i in indices:
            vx, vy = verts[i]
            wx, wy = self.mesh_to_world(obj, vx, vy)
            wx += dx
            wy += dy
            if self.snap_to_grid:
                wx = self.snap_value(wx, self.move_step)
                wy = self.snap_value(wy, self.move_step)
            verts[i] = self.world_to_mesh(obj, wx, wy)
        self.draw_scene(update_list=False)

    def resizeEvent(self, ev):  # pragma: no cover - gui layout
        super().resizeEvent(ev)
        try:
            h = self.viewport.height()
            if hasattr(self.mode_bar, "setFixedHeight"):
                self.mode_bar.setFixedHeight(h)
            self._reposition_preview()
            self._update_rulers()
        except Exception:
            log.exception("Failed to update toolbar height")

    def _reposition_preview(self) -> None:
        """Position the camera preview widget in the bottom-right corner."""
        preview = getattr(self, "preview_frame", None)
        if preview is None:
            preview = getattr(self, "preview_widget", None)
        view = getattr(self, "viewport", None)
        if not preview or not view:
            return
        if not hasattr(preview, "move") or not hasattr(view, "width"):
            return
        w = view.width() or 0
        h = view.height() or 0
        pw = preview.width() if hasattr(preview, "width") else 0
        ph = preview.height() if hasattr(preview, "height") else 0
        preview.move(w - pw - 10, h - ph - 10)

    def show_camera_preview(self, cam: Camera) -> None:
        """Display a preview from *cam* in the corner of the viewport."""
        self.preview_camera = cam
        if self.preview_renderer is None:
            try:
                try:
                    import viewport as _vp  # type: ignore
                except Exception:
                    from sage_editor.plugins import viewport as _vp
                self.preview_renderer = _vp.OpenGLRenderer(
                    width=160,
                    height=120,
                    widget=self.preview_widget,
                    vsync=False,
                    keep_aspect=True,
                )
            except Exception:
                log.exception("Failed to create preview renderer")
                return
        else:
            try:
                self.preview_renderer.set_window_size(160, 120)
            except Exception:
                pass
        if hasattr(self.preview_frame, "show"):
            self.preview_frame.show()
        elif hasattr(self.preview_widget, "show"):
            self.preview_widget.show()
        self._reposition_preview()
        self.preview_renderer.draw_scene(self.scene, cam)

    def hide_camera_preview(self) -> None:
        self.preview_camera = None
        if hasattr(self.preview_frame, "hide"):
            self.preview_frame.hide()
        elif hasattr(self.preview_widget, "hide"):
            self.preview_widget.hide()

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
            QCheckBox,
            QFileDialog,
            QHBoxLayout,
        )  # type: ignore[import-not-found]

        dlg = QDialog(self)
        dlg.setWindowTitle("Save Screenshot")
        form = QFormLayout(dlg)

        path_edit = NoWheelLineEdit("screenshot.png", dlg)
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
        if hasattr(grid_check, "setTextInteractionFlags"):
            flag = getattr(Qt, "TextInteractionFlag", None)
            if flag is not None and hasattr(flag, "NoTextInteraction"):
                grid_check.setTextInteractionFlags(flag.NoTextInteraction)
            else:
                no_flag = getattr(Qt, "NoTextInteraction", None)
                if no_flag is not None:
                    grid_check.setTextInteractionFlags(no_flag)
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

    def open_snap_dialog(self) -> None:
        """Backward compatible wrapper for opening the snap dock."""
        self.open_snap_dock()

    def load_project(self, path: str) -> None:
        from engine.core.project import Project
        from engine.core.resources import set_resource_root

        proj = Project.load(path)
        base = os.path.dirname(path)
        root = os.path.join(base, proj.resources)
        set_resource_root(root)
        self.resource_root = os.path.abspath(root)
        self.resources_label.setText(f"Resources: {self.resource_root}")
        self.scene = Scene.from_dict(proj.scene, base_path=base)
        self.camera = Camera(width=proj.width, height=proj.height, active=True)
        if hasattr(self.renderer, "set_window_size"):
            self.renderer.set_window_size(proj.width, proj.height)
        self.project_path = path
        self.update_object_list()
        self.hide_camera_preview()
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

    def undo(self) -> None:
        """Undo the last operation."""
        self.undo_stack.undo(self)

    def redo(self) -> None:
        """Redo the previously undone operation."""
        self.undo_stack.redo(self)

    # coordinate helpers -------------------------------------------------

    def screen_to_world(self, point):
        w = self.viewport.width() or 1
        h = self.viewport.height() or 1
        cam = self.camera
        cam_zoom = getattr(cam, "zoom", 1.0)
        scale = units.UNITS_PER_METER
        sign = -1 if units.Y_UP else 1
        x = cam.x + (point.x() - w / 2) / (scale * cam_zoom)
        y = cam.y + (point.y() - h / 2) * sign / (scale * cam_zoom)
        return x, y

    def world_to_screen(self, pos: tuple[float, float]):
        """Convert world coordinates to viewport pixels."""
        x, y = pos
        w = self.viewport.width() or 1
        h = self.viewport.height() or 1
        cam = self.camera
        cam_zoom = getattr(cam, "zoom", 1.0)
        scale = units.UNITS_PER_METER
        sign = -1 if units.Y_UP else 1
        sx = (x - cam.x) * scale * cam_zoom + w / 2
        sy = (y - cam.y) * scale * cam_zoom * sign + h / 2
        return sx, sy

    def snap_value(self, value: float, step: float) -> float:
        if step > 0:
            return round(value / step) * step
        return value

    def _update_rulers(self) -> None:
        """Update ruler markers based on the camera and cursor."""
        h_ruler = getattr(self.viewport_container, "h_ruler", None)
        v_ruler = getattr(self.viewport_container, "v_ruler", None)
        if h_ruler is None or v_ruler is None:
            return
        cam = self.camera
        w = self.viewport.width() or 1
        h = self.viewport.height() or 1
        zoom = getattr(cam, "zoom", 1.0)
        scale = units.UNITS_PER_METER * zoom
        sign = -1 if units.Y_UP else 1
        left = cam.x - w / (2 * scale)
        top = cam.y - sign * h / (2 * scale)
        cx = self.cursor_pos[0] if self.cursor_pos else None
        cy = self.cursor_pos[1] if self.cursor_pos else None
        if hasattr(h_ruler, "set_transform"):
            h_ruler.set_transform(left, scale, cx)
        if hasattr(v_ruler, "set_transform"):
            v_ruler.set_transform(top, scale, cy, sign)

    def mesh_to_world(self, obj: GameObject, vx: float, vy: float) -> tuple[float, float]:
        """Return world coordinates for a mesh vertex."""
        w = obj.width * obj.scale_x
        h = obj.height * obj.scale_y
        px = w * obj.pivot_x
        py = h * obj.pivot_y
        off_x = px - w / 2
        off_y = py - h / 2
        sx = -1 if obj.flip_x else 1
        sy = -1 if obj.flip_y else 1
        vx = (vx * w - off_x) * sx + off_x
        vy = (vy * h - off_y) * sy + off_y
        ang = math.radians(obj.angle)
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        rx = vx * cos_a - vy * sin_a
        ry = vx * sin_a + vy * cos_a
        return obj.x + rx, obj.y + ry

    def world_to_mesh(self, obj: GameObject, x: float, y: float) -> tuple[float, float]:
        """Convert world coordinates back to mesh space."""
        w = obj.width * obj.scale_x
        h = obj.height * obj.scale_y
        px = w * obj.pivot_x
        py = h * obj.pivot_y
        off_x = px - w / 2
        off_y = py - h / 2
        sx = -1 if obj.flip_x else 1
        sy = -1 if obj.flip_y else 1
        ang = math.radians(obj.angle)
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        rx = x - obj.x
        ry = y - obj.y
        vx = rx * cos_a + ry * sin_a
        vy = -rx * sin_a + ry * cos_a
        vx = (vx - off_x) / sx + off_x
        vy = (vy - off_y) / sy + off_y
        return vx / w if w else 0.0, vy / h if h else 0.0

    def _vertex_normal(self, verts: list[tuple[float, float]], i: int) -> tuple[float, float]:
        """Return a unit normal for vertex ``i`` in mesh space."""
        orient = self._polygon_orientation(verts)
        prev = verts[i - 1]
        cur = verts[i]
        nxt = verts[(i + 1) % len(verts)]
        dx1 = cur[0] - prev[0]
        dy1 = cur[1] - prev[1]
        dx2 = nxt[0] - cur[0]
        dy2 = nxt[1] - cur[1]
        n1 = (dy1, -dx1)
        n2 = (dy2, -dx2)
        len1 = math.hypot(*n1) or 1.0
        len2 = math.hypot(*n2) or 1.0
        n1x = n1[0] / len1
        n1y = n1[1] / len1
        n2x = n2[0] / len2
        n2y = n2[1] / len2
        dot = n1x * n2x + n1y * n2y
        if abs(dot) > 0.8:
            nx, ny = n1x, n1y
        else:
            nx = n1x + n2x
            ny = n1y + n2y
            length = math.hypot(nx, ny) or 1.0
            nx /= length
            ny /= length
        if (dx1 * dy2 - dy1 * dx2) * orient < 0:
            nx = -nx
            ny = -ny
        return nx, ny

    def _edge_normal(self, verts: list[tuple[float, float]], i: int) -> tuple[float, float]:
        """Return a unit normal for edge ``i`` in mesh space."""
        orient = self._polygon_orientation(verts)
        a = verts[i]
        b = verts[(i + 1) % len(verts)]
        dx = b[0] - a[0]
        dy = b[1] - a[1]
        nx, ny = dy, -dx
        length = math.hypot(nx, ny) or 1.0
        nx /= length
        ny /= length
        if orient < 0:
            nx = -nx
            ny = -ny
        return nx, ny

    def _polygon_orientation(self, verts: list[tuple[float, float]]) -> int:
        area = 0.0
        for i, (x0, y0) in enumerate(verts):
            x1, y1 = verts[(i + 1) % len(verts)]
            area += x0 * y1 - y0 * x1
        return 1 if area >= 0 else -1

    def set_renderer(self, renderer):
        self.viewport.renderer = renderer

    def change_renderer(self, backend: str) -> None:
        try:
            import viewport as _vp  # type: ignore
        except Exception:
            from sage_editor.plugins import viewport as _vp
        if backend == "opengl":
            rcls = _vp.OpenGLRenderer
        else:
            from engine.renderers import get_renderer
            rcls = get_renderer(backend)
            if rcls is None:
                self.log_warning(f"Renderer '{backend}' unavailable; falling back to OpenGL")
                rcls = _vp.OpenGLRenderer
        if self.renderer is not None:
            try:
                self.renderer.close()
            except Exception:
                log.exception("Renderer close failed")
        old_view = self.viewport
        w = self.viewport.width() or 640
        h = self.viewport.height() or 480
        if (backend == "sdl" and not isinstance(self.viewport, SDLViewportWidget)) or (
            backend != "sdl" and isinstance(self.viewport, SDLViewportWidget)
        ):
            if old_view is not None and hasattr(old_view, "deleteLater"):
                old_view.deleteLater()
            new_view = self._create_viewport_widget(backend)
            self.setCentralWidget(new_view)
            self.viewport_container = new_view
            self.viewport = new_view.viewport  # type: ignore[attr-defined]
            self.mode_bar = new_view.mode_bar  # type: ignore[attr-defined]
            self.cursor_label = new_view.cursor_label  # type: ignore[attr-defined]
            self.preview_widget = new_view.preview_widget  # type: ignore[attr-defined]
            self.preview_frame = new_view.preview_frame  # type: ignore[attr-defined]
        self.renderer = rcls(width=w, height=h, widget=self.viewport, vsync=False, keep_aspect=False)
        self.renderer_backend = backend if rcls is not _vp.OpenGLRenderer else "opengl"
        self.set_renderer(self.renderer)
        self.draw_scene()

    def set_objects(self, names):
        self.objects.clear()
        self.objects.addItems(list(names))

    def _object_path(self, obj: GameObject) -> str:
        parts = [obj.name]
        parent = getattr(obj, "parent", None)
        while parent is not None:
            parts.append(parent.name)
            parent = getattr(parent, "parent", None)
        return "/".join(reversed(parts))

    def update_object_list(self, preserve: bool = True):
        current = self.selected_obj.name if preserve and self.selected_obj else None
        names = [self._object_path(o) for o in self.scene.objects]
        self.set_objects(names)
        selected_names = {o.name for o in self.selected_objs}
        for i in range(self.objects.count()):
            item = self.objects.item(i)
            name = item.text().split("/")[-1]
            if hasattr(item, "setSelected"):
                item.setSelected(name in selected_names)
            if name == current:
                self.objects.setCurrentItem(item)

    def update_properties(self):
        self._updating_properties = True
        self.properties.set_object(self.selected_obj)
        self._updating_properties = False

    def apply_properties(self, update_list: bool = True) -> None:
        if getattr(self, "_updating_properties", False):
            return
        if self.selected_obj is None:
            return
        try:
            self.properties.apply_to_object(self.selected_obj)
        except Exception:
            log.exception("Failed to apply properties")
        # Refresh the UI so role-specific categories appear immediately
        self.update_properties()
        self.draw_scene(update_list=update_list)

    def find_object_at(self, x: float, y: float) -> Optional[GameObject]:
        for obj in reversed(self.scene.objects):
            if isinstance(obj, Camera):
                continue
            left, bottom, w, h = obj.rect()
            if left <= x <= left + w and bottom <= y <= bottom + h:
                return obj
        return None

    def select_object(self, obj: Optional[GameObject], additive: bool = False) -> None:
        if not additive:
            self.selected_objs = [obj] if obj else []
            self.selected_vertices.clear()
            self.selected_edges.clear()
            self.selected_face = False
        else:
            if obj is None:
                return
            if obj in self.selected_objs:
                self.selected_objs.remove(obj)
            else:
                self.selected_objs.append(obj)
        self.selected_obj = self.selected_objs[-1] if self.selected_objs else None

        selected_names = {o.name for o in self.selected_objs}
        for i in range(self.objects.count()):
            item = self.objects.item(i)
            name = item.text().split("/")[-1]
            if hasattr(item, "setSelected"):
                item.setSelected(name in selected_names)
            if name == getattr(self.selected_obj, "name", None):
                self.objects.setCurrentItem(item)
        if not self.selected_obj:
            self.objects.setCurrentItem(None)

        self.update_properties()
        self.draw_scene(update_list=False)

    def select_vertex(self, index: int, additive: bool = False) -> None:
        if not additive:
            self.selected_vertices = {index}
        else:
            if index in self.selected_vertices:
                self.selected_vertices.remove(index)
            else:
                self.selected_vertices.add(index)
        verts = self.selected_obj.mesh.vertices if self.selected_obj else []
        if (
            len(self.selected_vertices) == 2
            and verts
            and all(0 <= i < len(verts) for i in self.selected_vertices)
        ):
            a, b = sorted(self.selected_vertices)
            if b == a + 1 or (a == len(verts) - 1 and b == 0):
                self.selected_edges = {a}
            else:
                self.selected_edges.clear()
        else:
            self.selected_edges.clear()
        self.draw_scene(update_list=False)

    def select_edge(self, index: int, additive: bool = False) -> None:
        if not additive:
            self.selected_edges = {index}
        else:
            if index in self.selected_edges:
                self.selected_edges.remove(index)
            else:
                self.selected_edges.add(index)
        self.draw_scene(update_list=False)

    def select_face(self, selected: bool) -> None:
        self.selected_face = selected
        self.draw_scene(update_list=False)

    def _update_selection_gizmo(self) -> None:
        """Refresh gizmo highlighting the currently selected object."""
        if hasattr(self.renderer, "clear_gizmos"):
            self.renderer.clear_gizmos()
        for obj in self.selected_objs:
            if isinstance(obj, Camera) or not hasattr(obj, "rect"):
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
            g = gizmos.polyline_gizmo(points, color=(1, 0.4, 0.2, 1), frames=None)
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
            if self.modeling and getattr(obj, "mesh", None) is not None:
                verts = [self.mesh_to_world(obj, vx, vy) for vx, vy in obj.mesh.vertices]
                for i, (wx, wy) in enumerate(verts):
                    color = (1.0, 0.8, 0.2, 1) if i in self.selected_vertices else (
                        0.2,
                        0.8,
                        1.0,
                        1,
                    )
                    self.renderer.add_gizmo(
                        gizmos.square_gizmo(
                            wx,
                            wy,
                            size=4,
                            color=color,
                            thickness=1,
                            frames=None,
                            filled=True,
                        )
                    )
                if len(verts) > 1:
                    color = (1.0, 0.8, 0.2, 1) if self.selected_face else (0.4, 1.0, 0.4, 1)
                    self.renderer.add_gizmo(
                        gizmos.polyline_gizmo(verts + [verts[0]], color=color, frames=None)
                    )
                    for i in self.selected_edges:
                        j = (i + 1) % len(verts)
                        self.renderer.add_gizmo(
                            gizmos.polyline_gizmo(
                                [verts[i], verts[j]],
                                color=(1.0, 0.8, 0.2, 1),
                                frames=None,
                            )
                        )
                    for i, (vx, vy) in enumerate(obj.mesh.vertices):
                        nx, ny = self._vertex_normal(obj.mesh.vertices, i)
                        wx, wy = verts[i]
                        wx2, wy2 = self.mesh_to_world(obj, vx + nx * 0.05, vy + ny * 0.05)
                        self.renderer.add_gizmo(
                            gizmos.polyline_gizmo(
                                [(wx, wy), (wx2, wy2)],
                                color=(0.6, 0.6, 1.0, 1),
                                frames=None,
                            )
                        )
            if not self.modeling and self.transform_mode == "rect":
                handle_size = 5
                cam = self.camera
                cam_zoom = getattr(cam, "zoom", 1.0)
                rot_y = bottom + h + _ViewportMixin.ROTATE_OFFSET / (
                    units.UNITS_PER_METER * cam_zoom
                )
                self.renderer.add_gizmo(
                    gizmos.circle_gizmo(
                        obj.x,
                        rot_y,
                        size=handle_size,
                        color=(0.2, 0.8, 1.0, 1),
                        thickness=1,
                        frames=None,
                        filled=True,
                    )
                )
                for x, y in points[:-1]:
                    self.renderer.add_gizmo(
                        gizmos.square_gizmo(
                            x,
                            y,
                            size=handle_size,
                            color=(1.0, 0.6, 0.2, 1),
                            thickness=1,
                            frames=None,
                            filled=True,
                        )
                    )

    def draw_scene(self, update_list: bool = True) -> None:
        """Render the current scene and refresh selection gizmos."""
        self._update_selection_gizmo()
        if update_list:
            self.update_object_list()
        try:
            self.renderer.draw_scene(
                self.scene,
                self.camera,
                selected=self.selected_obj,
                mode=(self.transform_mode if not self.modeling else "pan"),
                cursor=self.cursor_pos,
                local=self.local_coords,
            )
        except TypeError:
            self.renderer.draw_scene(self.scene, self.camera)
        if self.preview_renderer and self.preview_camera:
            self.preview_renderer.draw_scene(self.scene, self.preview_camera)
        self._update_rulers()

    def start_game(self):
        from engine.core.engine import Engine
        from engine.game_window import GameWindow
        self.close_game()
        w = self.renderer.width
        h = self.renderer.height
        cam = self.scene.ensure_active_camera(w, h)
        self._engine = Engine(
            width=w,
            height=h,
            scene=self.scene,
            camera=cam,
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
        self.undo_stack.snapshot(self.scene)
        count = len([o for o in self.scene.objects if not isinstance(o, Camera)])
        obj = GameObject(name=f"Object {count}")
        obj.transform.x = x
        obj.transform.y = y
        self.scene.add_object(obj)
        self.update_object_list()
        self.draw_scene()
        return obj

    def create_empty(self, x: float = 0.0, y: float = 0.0) -> GameObject:
        self.undo_stack.snapshot(self.scene)
        count = len([o for o in self.scene.objects if getattr(o, "role", "") == "empty"])
        obj = GameObject(role="empty", name=f"Empty {count}")
        obj.transform.x = x
        obj.transform.y = y
        obj.width = 0
        obj.height = 0
        obj.visible = False
        self.scene.add_object(obj)
        self.update_object_list()
        self.draw_scene()
        return obj

    def create_camera(self, x: float = 0.0, y: float = 0.0) -> Camera:
        self.undo_stack.snapshot(self.scene)
        count = len([o for o in self.scene.objects if isinstance(o, Camera)])
        cam = Camera(x=x, y=y, active=False, name=f"Camera {count}")
        self.scene.add_object(cam)
        self.update_object_list()
        self.draw_scene()
        return cam

    def create_shape(self, shape: str, x: float = 0.0, y: float = 0.0) -> GameObject:
        """Create a basic shape object and add it to the scene."""
        self.undo_stack.snapshot(self.scene)
        count = len([o for o in self.scene.objects if getattr(o, "role", "") == "shape"])
        obj = GameObject(role="shape", name=f"{shape.capitalize()} {count}")
        obj.transform.x = x
        obj.transform.y = y
        obj.filled = True
        from engine.mesh_utils import (
            create_square_mesh,
            create_triangle_mesh,
            create_circle_mesh,
        )
        if shape == "triangle":
            obj.mesh = create_triangle_mesh()
        elif shape == "circle":
            obj.mesh = create_circle_mesh()
        elif shape == "polygon":
            from engine.mesh_utils import create_polygon_mesh
            verts = [
                (
                    math.cos(2 * math.pi * i / 5) * 1.0,
                    math.sin(2 * math.pi * i / 5) * 1.0,
                )
                for i in range(5)
            ]
            obj.mesh = create_polygon_mesh(verts)
        else:
            obj.mesh = create_square_mesh()
        obj.width = 2.0
        obj.height = 2.0
        self.scene.add_object(obj)
        self.update_object_list()
        self.draw_scene()
        return obj

    # clipboard and selection helpers ----------------------------------

    def copy_selected(self) -> None:
        if self.selected_obj is None:
            return
        from engine.core.objects import object_to_dict
        data = object_to_dict(self.selected_obj)
        if data is None:
            self._clipboard = deepcopy(self.selected_obj)
        else:
            self._clipboard = data

    def paste_object(self) -> Optional[GameObject]:
        if not self._clipboard:
            return None
        self.undo_stack.snapshot(self.scene)
        if isinstance(self._clipboard, dict):
            from engine.core.objects import object_from_dict
            try:
                obj = cast(GameObject, object_from_dict(dict(self._clipboard)))
            except Exception:
                log.exception("Failed to paste object")
                return None
        else:
            obj = deepcopy(cast(GameObject, self._clipboard))
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
        self.undo_stack.snapshot(self.scene)
        self.scene.remove_object(self.selected_obj)
        self.selected_obj = None
        self.update_object_list()
        self.draw_scene()

    def _list_context_menu(self, point):
        menu = QMenu(self.objects)
        if self.selected_obj is not None:
            copy_a = menu.addAction("Copy") if hasattr(menu, "addAction") else None
            paste_a = menu.addAction("Paste") if hasattr(menu, "addAction") else None
            del_a = menu.addAction("Delete") if hasattr(menu, "addAction") else None
            if copy_a is not None and hasattr(copy_a, "triggered"):
                copy_a.triggered.connect(self.copy_selected)
            if paste_a is not None and hasattr(paste_a, "triggered"):
                paste_a.triggered.connect(self.paste_object)
            if del_a is not None and hasattr(del_a, "triggered"):
                del_a.triggered.connect(self.delete_selected)
            if hasattr(menu, "addSeparator"):
                menu.addSeparator()
        sprite_a = menu.addAction("Create Sprite") if hasattr(menu, "addAction") else None
        empty_a = menu.addAction("Create Empty") if hasattr(menu, "addAction") else None
        cam_a = menu.addAction("Create Camera") if hasattr(menu, "addAction") else None
        shape_m = menu.addMenu("Create Shape") if hasattr(menu, "addMenu") else None
        if sprite_a is not None and hasattr(sprite_a, "triggered"):
            sprite_a.triggered.connect(self.create_object)
        if empty_a is not None and hasattr(empty_a, "triggered"):
            empty_a.triggered.connect(self.create_empty)
        if cam_a is not None and hasattr(cam_a, "triggered"):
            cam_a.triggered.connect(self.create_camera)
        if shape_m is not None:
            sq_a = shape_m.addAction("Square") if hasattr(shape_m, "addAction") else None
            tri_a = shape_m.addAction("Triangle") if hasattr(shape_m, "addAction") else None
            cir_a = shape_m.addAction("Circle") if hasattr(shape_m, "addAction") else None
            poly_a = shape_m.addAction("Polygon") if hasattr(shape_m, "addAction") else None
            if sq_a is not None and hasattr(sq_a, "triggered"):
                sq_a.triggered.connect(lambda: self.create_shape("square"))
            if tri_a is not None and hasattr(tri_a, "triggered"):
                tri_a.triggered.connect(lambda: self.create_shape("triangle"))
            if cir_a is not None and hasattr(cir_a, "triggered"):
                cir_a.triggered.connect(lambda: self.create_shape("circle"))
            if poly_a is not None and hasattr(poly_a, "triggered"):
                poly_a.triggered.connect(lambda: self.create_shape("polygon"))
        menu.exec(self.objects.mapToGlobal(point))

    def _object_selected(self, current=None, _prev=None):
        if hasattr(self.objects, "selectedItems"):
            items = self.objects.selectedItems()
            names = [i.text().split("/")[-1] for i in items]
        elif current is not None:
            names = [current.text().split("/")[-1]] if current is not None else []
        elif hasattr(self.objects, "currentItem"):
            cur = self.objects.currentItem()
            names = [cur.text().split("/")[-1]] if cur is not None else []
        else:
            names = []
        self.selected_objs = [o for o in (self.scene.find_object(n) for n in names) if o]
        self.selected_obj = self.selected_objs[-1] if self.selected_objs else None
        if isinstance(self.selected_obj, Camera):
            self.show_camera_preview(cast(Camera, self.selected_obj))
        else:
            self.hide_camera_preview()
        self.update_properties()
        self.draw_scene(update_list=False)

    def closeEvent(self, event):  # pragma: no cover - gui cleanup
        if self.renderer is not None:
            try:
                self.renderer.close()
            except Exception:
                log.exception("Renderer close failed")
        from engine.utils.log import logger
        if getattr(self, "_console_handler", None) is not None:
            try:
                logger.removeHandler(self._console_handler)
            except Exception:
                pass
        self.close_game()
        base = super()
        if hasattr(base, "closeEvent"):
            base.closeEvent(event)


