"""Qt editor window with dockable viewport, object list and console.

The widget preview uses :class:`~engine.renderers.opengl.core.OpenGLRenderer` so
objects are drawn with hardware acceleration by default.
"""

from __future__ import annotations

import logging
import os
import math
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
    QScrollArea,
    QLabel,
    QLineEdit,
    QDial,
    QCheckBox,
    QComboBox,
    QWidget,
    QStyleFactory,
)
try:  # optional for tests
    from PyQt6.QtWidgets import QFrame  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback when QFrame missing
    QFrame = QWidget  # type: ignore[misc]
from PyQt6.QtGui import QAction, QKeySequence  # type: ignore[import-not-found]

try:  # support minimal test stubs
    from PyQt6.QtWidgets import QTextEdit  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback when QTextEdit missing
    QTextEdit = QPlainTextEdit
from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
from typing import Optional, cast, Any
from engine.utils import units

from engine.renderers.opengl.core import OpenGLRenderer
from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.entities.game_object import GameObject
from engine import gizmos

from sage_editor.qt import GLWidget, SDLWidget


class ConsoleHandler(logging.Handler):
    """Forward log records to a text widget."""

    def __init__(self, widget: object) -> None:
        super().__init__()
        self.widget: Any = widget

    def emit(self, record: logging.LogRecord) -> None:  # pragma: no cover - UI handler
        msg = self.format(record)
        color = "#dddddd"
        if record.levelno >= logging.ERROR:
            color = "#ff5555"
        elif record.levelno >= logging.WARNING:
            color = "#ffaa00"
        if hasattr(self.widget, "appendHtml"):
            self.widget.appendHtml(f"<span style='color:{color}'>{msg}</span>")
        elif hasattr(self.widget, "append"):
            self.widget.append(msg)
        else:
            text = getattr(self.widget, "toPlainText", lambda: "")()
            if hasattr(self.widget, "setPlainText"):
                self.widget.setPlainText(text + msg + "\n")


class TransformBar(QWidget):
    """Simple vertical bar with toggle buttons for transform modes."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(0, 0, 0, 0)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(2)
        self.move_btn = QPushButton("Move", self)
        self.rotate_btn = QPushButton("Rotate", self)
        self.scale_btn = QPushButton("Scale", self)
        self.rect_btn = QPushButton("Rect", self)
        self.local_btn = QPushButton("Local", self)
        for btn in [
            self.move_btn,
            self.rotate_btn,
            self.scale_btn,
            self.rect_btn,
            self.local_btn,
        ]:
            if hasattr(btn, "setCheckable"):
                btn.setCheckable(True)
            layout.addWidget(btn)
        if hasattr(layout, "addStretch"):
            layout.addStretch()


class AngleDial(QDial):
    """Dial that displays the current value inside the circle."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._label = QLabel(self)
        if hasattr(self._label, "setAlignment"):
            self._label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.valueChanged.connect(self._update_label)
        self._update_label(self.value())

    def resizeEvent(self, event):  # pragma: no cover - gui adjustment
        super().resizeEvent(event)
        if hasattr(self._label, "setGeometry"):
            self._label.setGeometry(0, 0, self.width(), self.height())

    def _update_label(self, value: int) -> None:
        if hasattr(self._label, "setText"):
            self._label.setText(str(value))

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


def apply_fusion_style(app: QApplication) -> None:
    """Use the Qt Fusion style with a dark palette on *app*."""
    if hasattr(QApplication, "setStyle"):
        QApplication.setStyle(QStyleFactory.create("Fusion"))
        _apply_dark_palette(app)
    font = app.font()
    if font.pointSize() < 10:
        font.setPointSize(10)
        app.setFont(font)


class _ViewportMixin:
    """Input helpers shared between OpenGL and SDL widgets."""

    DRAG_THRESHOLD = 5
    HANDLE_PIXELS = 8
    ROTATE_OFFSET = 15

    def __init__(self, window: "EditorWindow", *a, **k) -> None:
        """Initialise mixin state without touching QWidget internals."""
        self._window = window
        self._last_pos = None
        self._press_pos = None
        self._dragging = False
        self._drag_mode: str | None = None
        self._drag_corner: str | None = None
        self._last_world: tuple[float, float] | None = None
        self._drag_angle: float | None = None
        if hasattr(self, "setContextMenuPolicy"):
            cast(QWidget, self).setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        if hasattr(self, "customContextMenuRequested"):
            cast(QWidget, self).customContextMenuRequested.connect(self._context_menu)

    # gizmo hit testing helpers --------------------------------------

    def _hit_move_handle(self, pos) -> str | None:
        obj = self._window.selected_obj
        if obj is None:
            return None
        sx, sy = self._window.world_to_screen((obj.x, obj.y))
        size = 50
        off = size * 0.3
        sq = 6
        th = self.HANDLE_PIXELS
        sign = -1 if units.Y_UP else 1
        dx = pos.x() - sx
        dy = pos.y() - sy
        if abs(dx - off) <= sq and abs(dy - sign * off) <= sq:
            return "move_xy"
        if abs(dy) <= th and 0 <= dx <= size:
            return "move_x"
        if abs(dx) <= th and 0 <= dy * sign <= size:
            return "move_y"
        return None

    def _hit_scale_handle(self, pos) -> str | None:
        obj = self._window.selected_obj
        if obj is None:
            return None
        sx, sy = self._window.world_to_screen((obj.x, obj.y))
        size = 50
        sq = 8
        th = self.HANDLE_PIXELS
        sign = -1 if units.Y_UP else 1
        dx = pos.x() - sx
        dy = pos.y() - sy
        ang = math.radians(getattr(obj, "angle", 0.0))
        ca = math.cos(ang)
        sa = math.sin(ang)
        lx = dx * ca + dy * sa
        ly = -dx * sa + dy * ca
        if abs(ly) <= th and abs(lx - size) <= sq:
            return "scale_x"
        if abs(lx) <= th and abs(ly - sign * size) <= sq:
            return "scale_y"
        return None

    def _hit_rotate_handle(self, pos) -> bool:
        obj = self._window.selected_obj
        if obj is None:
            return False
        sx, sy = self._window.world_to_screen((obj.x, obj.y))
        ring = 60
        dx = pos.x() - sx
        dy = pos.y() - sy
        dist = math.hypot(dx, dy)
        return abs(dist - ring) <= self.HANDLE_PIXELS

    # event handlers are same as before
    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            self._last_pos = ev.position() if hasattr(ev, "position") else ev.pos()
            self._press_pos = self._last_pos
            self._dragging = False
            self._drag_mode = "pan"
            self._drag_corner = None
            self._last_world = None
            obj = self._window.selected_obj
            if obj is not None and self._window.transform_mode == "rect":
                wx, wy = self._window.screen_to_world(self._press_pos)
                if isinstance(obj, Camera):
                    left, bottom, w, h = obj.view_rect()
                else:
                    left, bottom, w, h = obj.rect()
                cam = self._window.camera
                th = self.HANDLE_PIXELS / (units.UNITS_PER_METER * cam.zoom)
                corners = {
                    "bl": (left, bottom),
                    "br": (left + w, bottom),
                    "tl": (left, bottom + h),
                    "tr": (left + w, bottom + h),
                }
                rot_y = bottom + h + _ViewportMixin.ROTATE_OFFSET / (
                    units.UNITS_PER_METER * cam.zoom
                )
                rot_pos = (obj.x, rot_y)
                for name, (cx, cy) in corners.items():
                    if abs(wx - cx) <= th and abs(wy - cy) <= th:
                        self._drag_mode = "resize"
                        self._drag_corner = name
                        self._last_world = (wx, wy)
                        break
                else:
                    if abs(wx - rot_pos[0]) <= th and abs(wy - rot_pos[1]) <= th:
                        self._drag_mode = "rotate"
                        self._drag_corner = "rot"
                        self._last_world = (wx, wy)
                    elif left <= wx <= left + w and bottom <= wy <= bottom + h:
                        self._drag_mode = "move"
                        self._last_world = (wx, wy)
            elif obj is not None and self._window.transform_mode == "move":
                handle = self._hit_move_handle(self._press_pos)
                if handle:
                    self._drag_mode = handle
                    self._last_world = self._window.screen_to_world(self._press_pos)
            elif obj is not None and self._window.transform_mode == "scale":
                handle = self._hit_scale_handle(self._press_pos)
                if handle:
                    self._drag_mode = handle
                    self._last_world = self._window.screen_to_world(self._press_pos)
            elif obj is not None and self._window.transform_mode == "rotate":
                if self._hit_rotate_handle(self._press_pos):
                    self._drag_mode = "rotate"
                    self._last_world = self._window.screen_to_world(self._press_pos)
            try:
                from PyQt6.QtGui import QCursor  # type: ignore[import-not-found]
                cast(QWidget, self).setCursor(QCursor(Qt.CursorShape.ClosedHandCursor))
            except Exception:
                log.exception("Failed to set cursor")

    def mouseMoveEvent(self, ev):  # pragma: no cover - gui interaction
        pos = ev.position() if hasattr(ev, "position") else ev.pos()
        wx, wy = self._window.screen_to_world(pos)
        self._window.update_cursor(wx, wy)
        if self._press_pos is not None and ev.buttons() & Qt.MouseButton.LeftButton:
            if not self._dragging:
                dx = pos.x() - self._press_pos.x()
                dy = pos.y() - self._press_pos.y()
                if abs(dx) > self.DRAG_THRESHOLD or abs(dy) > self.DRAG_THRESHOLD:
                    self._dragging = True
                    self._last_pos = pos
            if self._dragging and self._last_pos is not None:
                cam = self._window.camera
                wx, wy = self._window.screen_to_world(pos)
                if self._drag_mode in ("move", "move_x", "move_y", "move_xy") and self._last_world is not None:
                    dx = wx - self._last_world[0]
                    dy = wy - self._last_world[1]
                    obj = self._window.selected_obj
                    if obj is not None:
                        if self._drag_mode in ("move", "move_xy"):
                            obj.x += dx
                            obj.y += dy
                        elif self._drag_mode == "move_x":
                            if self._window.local_coords:
                                ang = math.radians(getattr(obj, "angle", 0.0))
                                ca = math.cos(ang)
                                sa = math.sin(ang)
                                obj.x += dx * ca + dy * sa
                            else:
                                obj.x += dx
                        elif self._drag_mode == "move_y":
                            if self._window.local_coords:
                                ang = math.radians(getattr(obj, "angle", 0.0))
                                ca = math.cos(ang)
                                sa = math.sin(ang)
                                obj.y += -dx * sa + dy * ca
                            else:
                                obj.y += dy
                        self._window.update_properties()
                    self._last_world = (wx, wy)
                elif self._drag_mode in ("scale_x", "scale_y") and self._last_world is not None:
                    dx = wx - self._last_world[0]
                    dy = wy - self._last_world[1]
                    obj = self._window.selected_obj
                    if obj is not None:
                        if self._window.local_coords:
                            ang = math.radians(getattr(obj, "angle", 0.0))
                            ca = math.cos(ang)
                            sa = math.sin(ang)
                            local_dx = dx * ca + dy * sa
                            local_dy = -dx * sa + dy * ca
                            if self._drag_mode == "scale_x":
                                new_w = max(0.1, obj.width * obj.scale_x + local_dx)
                                obj.scale_x = new_w / obj.width
                            else:
                                new_h = max(0.1, obj.height * obj.scale_y + local_dy)
                                obj.scale_y = new_h / obj.height
                        else:
                            left, bottom, bw, bh = obj.rect()
                            ang = math.radians(getattr(obj, "angle", 0.0))
                            ca = abs(math.cos(ang))
                            sa = abs(math.sin(ang))
                            if self._drag_mode == "scale_x":
                                target = max(0.1, bw + dx)
                                if ca > 1e-6:
                                    new_sx = (target - sa * obj.height * obj.scale_y) / (ca * obj.width)
                                    obj.scale_x = max(new_sx, 0.01)
                                elif sa > 1e-6:
                                    new_sy = (target - ca * obj.width * obj.scale_x) / (sa * obj.height)
                                    obj.scale_y = max(new_sy, 0.01)
                            else:
                                target = max(0.1, bh + dy)
                                if ca > 1e-6:
                                    new_sy = (target - sa * obj.width * obj.scale_x) / (ca * obj.height)
                                    obj.scale_y = max(new_sy, 0.01)
                                elif sa > 1e-6:
                                    new_sx = (target - ca * obj.height * obj.scale_y) / (sa * obj.width)
                                    obj.scale_x = max(new_sx, 0.01)
                        
                        self._window.update_properties()
                    self._last_world = (wx, wy)
                elif self._drag_mode == "resize" and self._last_world is not None:
                    dx = wx - self._last_world[0]
                    dy = wy - self._last_world[1]
                    obj = self._window.selected_obj
                    if obj is not None:
                        w_old = obj.width * obj.scale_x
                        h_old = obj.height * obj.scale_y
                        left = obj.x - obj.width * obj.pivot_x * obj.scale_x
                        bottom = obj.y - obj.height * obj.pivot_y * obj.scale_y
                        w = w_old
                        h = h_old
                        shift = hasattr(ev, "modifiers") and (
                            ev.modifiers() & Qt.KeyboardModifier.ShiftModifier
                        )
                        if self._window.mirror_resize:
                            if shift:
                                aspect = w / h if h else 1
                                if abs(dx) >= abs(dy):
                                    delta_w = dx if (self._drag_corner and "r" in self._drag_corner) else -dx
                                    w += delta_w
                                    h = w / aspect
                                else:
                                    delta_h = dy if (self._drag_corner and "t" in self._drag_corner) else -dy
                                    h += delta_h
                                    w = h * aspect
                            else:
                                if self._drag_corner and "r" in self._drag_corner:
                                    w += dx
                                else:
                                    w -= dx
                                if self._drag_corner and "t" in self._drag_corner:
                                    h += dy
                                else:
                                    h -= dy
                            w = max(0.1, w)
                            h = max(0.1, h)
                            obj.scale_x = w / obj.width
                            obj.scale_y = h / obj.height
                            self._window.update_properties()
                            self._last_world = (wx, wy)
                        else:
                            if shift:
                                aspect = w / h if h else 1
                                if abs(dx) >= abs(dy):
                                    delta_w = dx if (self._drag_corner and "r" in self._drag_corner) else -dx
                                    w += delta_w
                                    h = w / aspect
                                else:
                                    delta_h = dy if (self._drag_corner and "t" in self._drag_corner) else -dy
                                    h += delta_h
                                    w = h * aspect
                            else:
                                if self._drag_corner and "r" in self._drag_corner:
                                    w += dx
                                else:
                                    w -= dx
                                if self._drag_corner and "t" in self._drag_corner:
                                    h += dy
                                else:
                                    h -= dy
                            new_left = left
                            new_bottom = bottom
                            if self._drag_corner and "l" in self._drag_corner:
                                new_left = left + (w_old - w)
                            if self._drag_corner and "b" in self._drag_corner:
                                new_bottom = bottom + (h_old - h)
                            w = max(0.1, w)
                            h = max(0.1, h)
                            obj.scale_x = w / obj.width
                            obj.scale_y = h / obj.height
                            obj.x = new_left + obj.pivot_x * w
                            obj.y = new_bottom + obj.pivot_y * h
                            self._window.update_properties()
                            self._last_world = (wx, wy)
                elif self._drag_mode == "rotate" and self._last_world is not None:
                    obj = self._window.selected_obj
                    if obj is not None:
                        cx, cy = obj.x, obj.y
                        ang0 = math.atan2(self._last_world[1] - cy, self._last_world[0] - cx)
                        ang1 = math.atan2(wy - cy, wx - cx)
                        obj.angle += math.degrees(ang1 - ang0)
                        self._window.update_properties()
                    self._last_world = (wx, wy)
                else:
                    dx = pos.x() - self._last_pos.x()
                    dy = pos.y() - self._last_pos.y()
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
            self._drag_mode = None
            self._drag_corner = None
            self._last_world = None
            self._drag_angle = None
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
            copy_a = menu.addAction("Copy") if hasattr(menu, "addAction") else None
            paste_a = menu.addAction("Paste") if hasattr(menu, "addAction") else None
            del_a = menu.addAction("Delete") if hasattr(menu, "addAction") else None
            if copy_a is not None and hasattr(copy_a, "triggered"):
                copy_a.triggered.connect(self._window.copy_selected)
            if paste_a is not None and hasattr(paste_a, "triggered"):
                paste_a.triggered.connect(self._window.paste_object)
            if del_a is not None and hasattr(del_a, "triggered"):
                del_a.triggered.connect(self._window.delete_selected)
        else:
            sprite_a = menu.addAction("Create Sprite") if hasattr(menu, "addAction") else None
            empty_a = menu.addAction("Create Empty") if hasattr(menu, "addAction") else None
            cam_a = menu.addAction("Create Camera") if hasattr(menu, "addAction") else None

            if sprite_a is not None and hasattr(sprite_a, "triggered"):
                sprite_a.triggered.connect(lambda: self._window.create_object(wx, wy))
            if empty_a is not None and hasattr(empty_a, "triggered"):
                empty_a.triggered.connect(lambda: self._window.create_empty(wx, wy))
            if cam_a is not None and hasattr(cam_a, "triggered"):
                cam_a.triggered.connect(lambda: self._window.create_camera(wx, wy))
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
        super().mousePressEvent(ev)

    def mouseMoveEvent(self, ev):  # pragma: no cover - gui interaction
        super().mouseMoveEvent(ev)

    def mouseReleaseEvent(self, ev):  # pragma: no cover - gui interaction
        super().mouseReleaseEvent(ev)

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



class PropertiesWidget(QWidget):
    """Widget for editing object properties."""

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)

        self.object_group = QGroupBox("Object", self)
        obj_form = QFormLayout(self.object_group)
        self.name_edit = QLineEdit(self)
        obj_form.addRow("Name", self.name_edit)
        self.role_combo = QComboBox(self)
        if hasattr(self.role_combo, "addItems"):
            self.role_combo.addItems(["empty", "sprite", "camera"])
        obj_form.addRow("Role", self.role_combo)
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

        self.rot_dial = AngleDial(self)
        self.rot_dial.setRange(0, 360)
        # Invert the dial so clockwise motion yields clockwise rotation
        if hasattr(self.rot_dial, "setInvertedAppearance"):
            self.rot_dial.setInvertedAppearance(True)
        if hasattr(self.rot_dial, "setInvertedControls"):
            self.rot_dial.setInvertedControls(True)
        trans_form.addRow("Rotation", self.rot_dial)

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

        try:
            from engine import physics  # noqa: F401
        except Exception:
            self.physics_group = None
        else:
            self.physics_group = QGroupBox("Physics", self)
            phys_form = QFormLayout(self.physics_group)
            self.physics_enabled = QCheckBox("Enabled", self)
            phys_form.addRow(self.physics_enabled)
            self.body_combo = QComboBox(self)
            if hasattr(self.body_combo, "addItems"):
                self.body_combo.addItems(["Dynamic", "Static"])
            phys_form.addRow("Body Type", self.body_combo)
            layout.addWidget(self.physics_group)

        layout.addStretch()

    def set_object(self, obj: Optional[GameObject]) -> None:
        if obj is None:
            self.name_edit.setText("")
            if hasattr(self, "role_combo"):
                self.role_combo.setCurrentIndex(-1)
            self.tags_edit.setText("")
            self.visible_check.setChecked(False)
            self.pos_x.setText("")
            self.pos_y.setText("")
            self.rot_dial.setValue(0)
            self.scale_x.setText("")
            self.scale_y.setText("")
            self.flip_x.setChecked(False)
            self.flip_y.setChecked(False)
            if hasattr(self.flip_x, "setEnabled"):
                self.flip_x.setEnabled(False)
            if hasattr(self.flip_y, "setEnabled"):
                self.flip_y.setEnabled(False)
            self.pivot_x.setText("")
            self.pivot_y.setText("")
            if getattr(self, "physics_group", None):
                self.physics_enabled.setChecked(False)
                if hasattr(self, "body_combo"):
                    self.body_combo.setCurrentIndex(0)
            return

        self.name_edit.setText(obj.name or "")
        role = getattr(obj, "role", "")
        if hasattr(self, "role_combo"):
            idx = self.role_combo.findText(role) if hasattr(self.role_combo, "findText") else -1
            if idx >= 0 and hasattr(self.role_combo, "setCurrentIndex"):
                self.role_combo.setCurrentIndex(idx)
            elif hasattr(self.role_combo, "setCurrentText"):
                self.role_combo.setCurrentText(role)
        tags = obj.metadata.get("tags", [])
        if isinstance(tags, (list, set)):
            tags = ",".join(tags)
        self.tags_edit.setText(str(tags))
        self.visible_check.setChecked(bool(getattr(obj, "visible", True)))
        self.pos_x.setText(str(getattr(obj, "x", 0.0)))
        self.pos_y.setText(str(getattr(obj, "y", 0.0)))
        angle = getattr(obj, "angle", 0.0)
        self.rot_dial.setValue(int((-angle) % 360))
        self.scale_x.setText(str(getattr(obj, "scale_x", 1.0)))
        self.scale_y.setText(str(getattr(obj, "scale_y", 1.0)))
        flip_allowed = role not in ("camera", "empty")
        if hasattr(self.flip_x, "setEnabled"):
            self.flip_x.setEnabled(flip_allowed)
        if hasattr(self.flip_y, "setEnabled"):
            self.flip_y.setEnabled(flip_allowed)
        self.flip_x.setChecked(flip_allowed and bool(getattr(obj, "flip_x", False)))
        self.flip_y.setChecked(flip_allowed and bool(getattr(obj, "flip_y", False)))
        self.pivot_x.setText(str(getattr(obj, "pivot_x", 0.0)))
        self.pivot_y.setText(str(getattr(obj, "pivot_y", 0.0)))
        if getattr(self, "physics_group", None):
            self.physics_enabled.setChecked(bool(getattr(obj, "physics_enabled", False)))
            body = getattr(obj, "body_type", "dynamic")
            idx = 0 if body != "static" else 1
            if hasattr(self, "body_combo"):
                self.body_combo.setCurrentIndex(idx)

    def apply_to_object(self, obj: GameObject) -> None:
        obj.name = self.name_edit.text()
        tags = [t.strip() for t in self.tags_edit.text().split(',') if t.strip()]
        if tags:
            obj.metadata["tags"] = tags
        obj.visible = self.visible_check.isChecked()
        if hasattr(self, "role_combo") and hasattr(self.role_combo, "currentText"):
            obj.role = self.role_combo.currentText() or obj.role
        try:
            obj.x = float(self.pos_x.text())
            obj.y = float(self.pos_y.text())
        except ValueError:
            log.warning("Invalid position")
        obj.angle = -float(self.rot_dial.value())
        try:
            sx = float(self.scale_x.text())
            sy = float(self.scale_y.text())
            if self.link_scale.isChecked():
                sy = sx
            obj.scale_x = sx
            obj.scale_y = sy
        except ValueError:
            log.warning("Invalid scale")
        if obj.role not in ("camera", "empty"):
            obj.flip_x = self.flip_x.isChecked()
            obj.flip_y = self.flip_y.isChecked()
        try:
            obj.pivot_x = float(self.pivot_x.text())
            obj.pivot_y = float(self.pivot_y.text())
        except ValueError:
            log.warning("Invalid pivot")
        if getattr(self, "physics_group", None):
            obj.physics_enabled = self.physics_enabled.isChecked()
            if hasattr(self, "body_combo"):
                obj.body_type = (
                    "static" if self.body_combo.currentIndex() == 1 else "dynamic"
                )

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
            view = SDLViewportWidget(self)
        else:
            view = ViewportWidget(self)
        container = QWidget(self)
        layout = QHBoxLayout(container)
        if hasattr(layout, "setContentsMargins"):
            # Small offset so the bar doesn't stick to the window frame
            layout.setContentsMargins(8, 8, 0, 0)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(4)
        bar = TransformBar(container)
        if hasattr(bar, "setSizePolicy"):
            pol = getattr(QSizePolicy, "Policy", QSizePolicy)
            horiz = getattr(pol, "Preferred", 0)
            vert = getattr(pol, "Expanding", 0)
            bar.setSizePolicy(horiz, vert)
        if hasattr(bar, "setFixedWidth"):
            bar.setFixedWidth(60)
        layout.addWidget(bar)
        layout.addWidget(view)
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
        label = QLabel("0, 0", container)
        if hasattr(label, "setObjectName"):
            label.setObjectName("CursorLabel")
        if hasattr(label, "setStyleSheet"):
            label.setStyleSheet("color:#ddd;background:rgba(0,0,0,0.5);padding:2px;")
        if hasattr(label, "move"):
            label.move(70, 8)
        container.cursor_label = label  # type: ignore[attr-defined]
        if hasattr(view, "setMouseTracking"):
            view.setMouseTracking(True)
        container.viewport = view  # type: ignore[attr-defined]
        container.mode_bar = bar  # type: ignore[attr-defined]
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
        if hasattr(self.mode_bar.move_btn, "setChecked"):
            self.mode_bar.move_btn.setChecked(True)
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
        for edit in [
            self.properties.name_edit,
            self.properties.tags_edit,
            self.properties.pos_x,
            self.properties.pos_y,
            self.properties.scale_x,
            self.properties.scale_y,
            self.properties.pivot_x,
            self.properties.pivot_y,
        ]:
            if hasattr(edit, "editingFinished"):
                edit.editingFinished.connect(self.apply_properties)
        for box in [
            self.properties.visible_check,
            self.properties.flip_x,
            self.properties.flip_y,
            getattr(self.properties, "physics_enabled", None),
        ]:
            if box is not None and hasattr(box, "stateChanged"):
                box.stateChanged.connect(lambda *_: self.apply_properties(False))
        if hasattr(self.properties.rot_dial, "valueChanged"):
            self.properties.rot_dial.valueChanged.connect(lambda *_: self.apply_properties(False))
        if hasattr(self.properties.role_combo, "currentIndexChanged"):
            self.properties.role_combo.currentIndexChanged.connect(lambda *_: self.apply_properties(False))
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
        self.mirror_resize = False
        self.local_coords = False
        self.transform_mode = "move"
        self.set_renderer(self.renderer)
        self.selected_obj: Optional[GameObject] = None
        self._clipboard: dict | None = None

        splitter = QSplitter(Qt.Orientation.Vertical, self)
        splitter.addWidget(self.viewport_container)
        splitter.addWidget(self.console)
        splitter.setStretchFactor(0, 5)
        splitter.setStretchFactor(1, 1)
        self._splitter = splitter
        self.setCentralWidget(splitter)

        menubar = QMenuBar(self)
        self.setMenuBar(menubar)
        file_menu = menubar.addMenu("File") if hasattr(menubar, "addMenu") else None
        open_p = file_menu.addAction("Open Project") if file_menu and hasattr(file_menu, "addAction") else None
        save_p = file_menu.addAction("Save Project") if file_menu and hasattr(file_menu, "addAction") else None
        shot_p = file_menu.addAction("Screenshot...") if file_menu and hasattr(file_menu, "addAction") else None
        edit_menu = menubar.addMenu("Edit") if hasattr(menubar, "addMenu") else None
        copy_m = edit_menu.addAction("Copy") if edit_menu and hasattr(edit_menu, "addAction") else None
        paste_m = edit_menu.addAction("Paste") if edit_menu and hasattr(edit_menu, "addAction") else None
        del_m = edit_menu.addAction("Delete") if edit_menu and hasattr(edit_menu, "addAction") else None
        self.copy_action = copy_m
        self.paste_action = paste_m
        self.delete_action = del_m
        if copy_m is not None and hasattr(copy_m, "setShortcut"):
            copy_m.setShortcut(QKeySequence("Ctrl+C"))
        if paste_m is not None and hasattr(paste_m, "setShortcut"):
            paste_m.setShortcut(QKeySequence("Ctrl+V"))
        if del_m is not None and hasattr(del_m, "setShortcut"):
            del_m.setShortcut(QKeySequence("Delete"))
        engine_menu = menubar.addMenu("Engine") if hasattr(menubar, "addMenu") else None
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
        view_menu = engine_menu.addMenu("View") if engine_menu and hasattr(engine_menu, "addMenu") else None
        grid_act = view_menu.addAction("Show Grid") if view_menu and hasattr(view_menu, "addAction") else None
        if grid_act is not None:
            if hasattr(grid_act, "setCheckable"):
                grid_act.setCheckable(True)
            if hasattr(grid_act, "setChecked"):
                grid_act.setChecked(True)
            if hasattr(grid_act, "triggered"):
                grid_act.triggered.connect(self.toggle_grid)

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

        menubar.addMenu("About")

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
        self.addToolBar(tbar)
        left_spacer = QWidget(self)
        left_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        tbar.addWidget(left_spacer)

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

        self.objects = QListWidget()
        obj_dock = QDockWidget("Objects", self)
        obj_dock.setObjectName("ObjectsDock")
        obj_dock.setWidget(self.objects)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)

        prop_dock = QDockWidget("Properties", self)
        prop_dock.setObjectName("PropertiesDock")
        prop_scroll = QScrollArea(self)
        prop_scroll.setWidgetResizable(True)
        prop_scroll.setWidget(self.properties)
        prop_dock.setWidget(prop_scroll)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, prop_dock)
        self.splitDockWidget(obj_dock, prop_dock, Qt.Orientation.Vertical)

        res_dock = QDockWidget("Resources", self)
        res_dock.setObjectName("ResourcesDock")
        res_dock.setWidget(res_container)
        self.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, res_dock)

        self.objects.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.objects.customContextMenuRequested.connect(self._list_context_menu)
        self.objects.currentItemChanged.connect(self._object_selected)

        self.selected_obj = None
        self.update_object_list()
        self._reposition_preview()
        self.draw_scene()

    def log_warning(self, text: str) -> None:
        """Display *text* in the console dock."""
        self.console.append(text)

    def toggle_grid(self, checked: bool) -> None:
        if hasattr(self.renderer, "show_grid"):
            self.renderer.show_grid = checked
            self.draw_scene(update_list=False)

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

    def update_cursor(self, x: float, y: float) -> None:
        """Store cursor world coordinates and update the overlay label."""
        self.cursor_pos = (x, y)
        if hasattr(self.cursor_label, "setText"):
            self.cursor_label.setText(f"{x:.1f}, {y:.1f}")
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

    def resizeEvent(self, ev):  # pragma: no cover - gui layout
        super().resizeEvent(ev)
        try:
            h = self.viewport.height()
            if hasattr(self.mode_bar, "setFixedHeight"):
                self.mode_bar.setFixedHeight(h)
            self._reposition_preview()
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
                self.preview_renderer = OpenGLRenderer(
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

    def world_to_screen(self, pos: tuple[float, float]):
        """Convert world coordinates to viewport pixels."""
        x, y = pos
        w = self.viewport.width() or 1
        h = self.viewport.height() or 1
        cam = self.camera
        scale = units.UNITS_PER_METER
        sign = -1 if units.Y_UP else 1
        sx = (x - cam.x) * scale * cam.zoom + w / 2
        sy = (y - cam.y) * scale * cam.zoom * sign + h / 2
        return sx, sy

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
            self.viewport_container = new_view
            self.viewport = new_view.viewport  # type: ignore[attr-defined]
            self.mode_bar = new_view.mode_bar  # type: ignore[attr-defined]
        self.renderer = rcls(width=w, height=h, widget=self.viewport, vsync=False, keep_aspect=False)
        self.renderer_backend = backend if rcls is not OpenGLRenderer else "opengl"
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
        if current:
            for i in range(self.objects.count()):
                item = self.objects.item(i)
                if item.text().split("/")[-1] == current:
                    self.objects.setCurrentItem(item)
                    break

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
        self.draw_scene(update_list=update_list)

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
                if item.text().split("/")[-1] == obj.name:
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
        if self.transform_mode == "rect":
            handle_size = 5
            cam = self.camera
            rot_y = bottom + h + _ViewportMixin.ROTATE_OFFSET / (
                units.UNITS_PER_METER * cam.zoom
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
                mode=self.transform_mode,
                cursor=self.cursor_pos,
                local=self.local_coords,
            )
        except TypeError:
            self.renderer.draw_scene(self.scene, self.camera)
        if self.preview_renderer and self.preview_camera:
            self.preview_renderer.draw_scene(self.scene, self.preview_camera)

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
        count = len([o for o in self.scene.objects if not isinstance(o, Camera)])
        obj = GameObject(name=f"Object {count}")
        obj.transform.x = x
        obj.transform.y = y
        self.scene.add_object(obj)
        self.update_object_list()
        self.draw_scene()
        return obj

    def create_empty(self, x: float = 0.0, y: float = 0.0) -> GameObject:
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
        count = len([o for o in self.scene.objects if isinstance(o, Camera)])
        cam = Camera(x=x, y=y, active=False, name=f"Camera {count}")
        self.scene.add_object(cam)
        self.update_object_list()
        self.draw_scene()
        return cam

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
        if sprite_a is not None and hasattr(sprite_a, "triggered"):
            sprite_a.triggered.connect(self.create_object)
        if empty_a is not None and hasattr(empty_a, "triggered"):
            empty_a.triggered.connect(self.create_empty)
        if cam_a is not None and hasattr(cam_a, "triggered"):
            cam_a.triggered.connect(self.create_camera)
        menu.exec(self.objects.mapToGlobal(point))

    def _object_selected(self, current, _prev):
        if current is not None:
            name = current.text().split("/")[-1]
            self.selected_obj = self.scene.find_object(name)
        else:
            self.selected_obj = None
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
    apply_fusion_style(app)

    window = EditorWindow(editor._menus, editor._toolbar)
    window.resize(800, 600)
    window.show()

    editor.window = window
    editor.viewport = window.viewport
    if created:
        app.exec()
