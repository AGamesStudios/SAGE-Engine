from __future__ import annotations

import math
from typing import TYPE_CHECKING, cast

from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
from PyQt6.QtWidgets import (
    QApplication,
    QWidget,
    QMenu,  # type: ignore[import-not-found]
)
from engine.core.camera import Camera
import logging
from engine.utils import units

log = logging.getLogger(__name__)

if TYPE_CHECKING:  # pragma: no cover - for type hints
    from sage_editor.plugins.editor_window import EditorWindow

ACCENT_COLOR = "#ffb84d"


def _apply_ember_palette(app: QApplication) -> None:
    """Apply the SAGE Ember palette with a soft yellow-orange accent."""
    from PyQt6.QtGui import QColor, QPalette  # type: ignore[import-not-found]

    accent = QColor(255, 184, 77)
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(45, 45, 45))
    palette.setColor(QPalette.ColorRole.WindowText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Base, QColor(35, 35, 35))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(55, 55, 55))
    palette.setColor(QPalette.ColorRole.ToolTipBase, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.ToolTipText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Text, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.Button, QColor(45, 45, 45))
    palette.setColor(QPalette.ColorRole.ButtonText, Qt.GlobalColor.white)
    palette.setColor(QPalette.ColorRole.BrightText, Qt.GlobalColor.red)
    palette.setColor(QPalette.ColorRole.Link, accent)
    palette.setColor(QPalette.ColorRole.Highlight, accent)
    palette.setColor(QPalette.ColorRole.HighlightedText, Qt.GlobalColor.black)
    app.setPalette(palette)


def _apply_ember_stylesheet(app: QApplication) -> None:
    """Style common widgets with an orange highlight."""
    stylesheet = f"""
    QPushButton, QToolButton {{
        background-color: #353535;
        border: none;
        padding: 4px 8px;
        margin: 2px;
        border-radius: 4px;
        color: white;
    }}
    QPushButton:hover, QToolButton:hover {{
        background-color: #474747;
    }}
    QPushButton:pressed, QToolButton:pressed {{
        background-color: {ACCENT_COLOR};
        color: black;
    }}
    QMenuBar {{
        background-color: #2c2c2c;
    }}
    QToolBar {{
        background-color: #2c2c2c;
        spacing: 4px;
    }}
    QToolBar::handle {{
        image: none;
        width: 0px;
        margin: 0px;
    }}
    QMenuBar::item {{
        padding: 4px 8px;
        margin: 0 2px;
        border-radius: 4px;
    }}
    QMenuBar::item:selected {{
        background-color: {ACCENT_COLOR};
        color: black;
    }}
    QMenu {{
        background-color: #2c2c2c;
        border: 1px solid #555555;
        border-radius: 6px;
        padding: 4px;
        margin: 2px; /* space from menu bar or parent menu */
    }}
    QMenu::item {{
        padding: 4px 12px;
        margin: 2px 0;
        border-radius: 4px;
    }}
    QMenu::item:selected {{
        background-color: {ACCENT_COLOR};
        color: black;
    }}
    QDockWidget::title {{
        background-color: #353535;
        text-align: center;
    }}
    QCheckBox::indicator, QRadioButton::indicator {{
        width: 12px;
        height: 12px;
        border: 1px solid #bbbbbb;
        border-radius: 3px;
        background: #353535;
    }}
    QCheckBox::indicator:hover, QRadioButton::indicator:hover {{
        border-color: {ACCENT_COLOR};
    }}
    QCheckBox::indicator:checked, QRadioButton::indicator:checked {{
        background-color: {ACCENT_COLOR};
        border: 1px solid {ACCENT_COLOR};
    }}
    QSlider::groove:horizontal {{
        height: 6px;
        background: #555555;
        border-radius: 3px;
    }}
    QSlider::handle:horizontal {{
        background: {ACCENT_COLOR};
        border: none;
        width: 12px;
        margin: -5px 0;
        border-radius: 6px;
    }}
    QProgressBar {{
        background-color: #353535;
        border: none;
        text-align: center;
    }}
    QProgressBar::chunk {{
        background-color: {ACCENT_COLOR};
    }}
    QComboBox {{
        background-color: #353535;
        border: none;
        padding: 2px 6px;
        border-radius: 4px;
    }}
    QComboBox::drop-down {{
        border: none;
    }}
    QComboBox QAbstractItemView {{
        background-color: #353535;
        border: 1px solid #555555;
        border-radius: 4px;
        selection-background-color: {ACCENT_COLOR};
        selection-color: black;
        padding: 2px;
        outline: 0;
    }}
    QComboBox QAbstractItemView::item {{
        padding: 2px 8px;
        margin: 2px 0;
        border-radius: 2px;
    }}
    QComboBox QAbstractItemView::item:selected {{
        background: {ACCENT_COLOR};
        color: black;
    }}
    QLineEdit, QPlainTextEdit {{
        background-color: #2b2b2b;
        border: 1px solid #555555;
        border-radius: 4px;
        padding: 2px;
        color: white;
    }}
    QLineEdit:focus, QPlainTextEdit:focus {{
        border-color: {ACCENT_COLOR};
    }}
    QScrollBar:vertical {{
        background: #2c2c2c;
        width: 12px;
        margin: 16px 0 16px 0;
    }}
    QScrollBar::handle:vertical {{
        background: #555555;
        min-height: 20px;
        border-radius: 5px;
    }}
    QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
        border: none;
        background: #2c2c2c;
    }}
    QScrollBar:horizontal {{
        background: #2c2c2c;
        height: 8px;
        margin: 0 16px 0 16px;
    }}
    QScrollBar::handle:horizontal {{
        background: #555555;
        min-width: 20px;
        border-radius: 5px;
    }}
    QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {{
        border: none;
        background: #2c2c2c;
    }}
    QSplitter::handle {{
        background: #2c2c2c;
        width: 4px;
    }}
    QListWidget::item:selected,
    QListView::item:selected,
    QTreeView::item:selected,
    QTableView::item:selected {{
        background: {ACCENT_COLOR};
        color: black;
    }}
    """
    app.setStyleSheet(stylesheet)


def apply_ember_style(app: QApplication) -> None:
    """Use the custom SAGE Ember style on *app*."""
    _apply_ember_palette(app)
    _apply_ember_stylesheet(app)
    font = app.font()
    if font.pointSize() < 12:
        font.setPointSize(12)
        app.setFont(font)


class _ViewportMixin:
    """Input helpers shared between OpenGL and SDL widgets."""

    DRAG_THRESHOLD = 5
    HANDLE_PIXELS = 8
    ROTATE_OFFSET = 15

    def __init__(self, window: "EditorWindow") -> None:
        """Initialise mixin state without touching QWidget internals."""
        self._window = window
        self._last_pos = None
        self._press_pos = None
        self._dragging = False
        self._drag_mode: str | None = None
        self._drag_corner: str | None = None
        self._drag_vertex: int | None = None
        self._last_world: tuple[float, float] | None = None
        self._drag_angle: float | None = None
        self._press_hit = None
        if hasattr(self, "setContextMenuPolicy"):
            cast(QWidget, self).setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        if hasattr(self, "customContextMenuRequested"):
            cast(QWidget, self).customContextMenuRequested.connect(self._context_menu)

    def _snap(self, value: float, step: float) -> float:
        if step > 0:
            return round(value / step) * step
        return value

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

    def _hit_vertex_handle(self, pos) -> int | None:
        obj = self._window.selected_obj
        if obj is None or not self._window.modeling or getattr(obj, "mesh", None) is None:
            return None
        th = self.HANDLE_PIXELS
        for i, (vx, vy) in enumerate(obj.mesh.vertices):
            wx, wy = self._window.mesh_to_world(obj, vx, vy)
            sx, sy = self._window.world_to_screen((wx, wy))
            if abs(pos.x() - sx) <= th and abs(pos.y() - sy) <= th:
                return i
        return None

    def _hit_edge_handle(self, pos) -> int | None:
        obj = self._window.selected_obj
        if obj is None or not self._window.modeling or getattr(obj, "mesh", None) is None:
            return None
        verts = obj.mesh.vertices
        wx, wy = self._window.screen_to_world(pos)
        th = self.HANDLE_PIXELS / (units.UNITS_PER_METER * self._window.camera.zoom)

        def _dist(px, py, ax, ay, bx, by):
            dx = bx - ax
            dy = by - ay
            if dx == dy == 0:
                return math.hypot(px - ax, py - ay)
            t = max(0.0, min(1.0, ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy)))
            ex = ax + t * dx
            ey = ay + t * dy
            return math.hypot(px - ex, py - ey)

        for i, (va, vb) in enumerate(zip(verts, verts[1:] + verts[:1])):
            ax, ay = self._window.mesh_to_world(obj, *va)
            bx, by = self._window.mesh_to_world(obj, *vb)
            if _dist(wx, wy, ax, ay, bx, by) <= th:
                return i
        return None

    def _hit_face_handle(self, pos) -> bool:
        obj = self._window.selected_obj
        if obj is None or not self._window.modeling or getattr(obj, "mesh", None) is None:
            return False
        wx, wy = self._window.screen_to_world(pos)
        mx, my = self._window.world_to_mesh(obj, wx, wy)
        verts = obj.mesh.vertices
        inside = False
        j = len(verts) - 1
        for i, (x0, y0) in enumerate(verts):
            x1, y1 = verts[j]
            if ((y0 > my) != (y1 > my)) and (
                mx < (x1 - x0) * (my - y0) / ((y1 - y0) or 1e-9) + x0
            ):
                inside = not inside
            j = i
        return inside

    # event handlers are same as before
    def mousePressEvent(self, ev):  # pragma: no cover - gui interaction
        if ev.button() == Qt.MouseButton.LeftButton:
            self._last_pos = ev.position() if hasattr(ev, "position") else ev.pos()
            self._press_pos = self._last_pos
            self._dragging = False
            self._drag_mode = "pan"
            self._drag_corner = None
            self._last_world = None
            wx, wy = self._window.screen_to_world(self._press_pos)
            self._press_hit = self._window.find_object_at(wx, wy)
            obj = self._window.selected_obj
            if obj is not None and self._window.modeling:
                mods = ev.modifiers() if hasattr(ev, "modifiers") else 0
                add = mods & (
                    Qt.KeyboardModifier.ControlModifier
                    | Qt.KeyboardModifier.ShiftModifier
                )
                if self._window.selection_mode == "vertex":
                    v = self._hit_vertex_handle(self._press_pos)
                    if v is not None:
                        self._window.select_vertex(v, additive=bool(add))
                        self._drag_mode = "vertex"
                        self._drag_vertex = v
                        self._last_world = self._window.screen_to_world(
                            self._press_pos
                        )
                        return
                elif self._window.selection_mode == "edge":
                    e = self._hit_edge_handle(self._press_pos)
                    if e is not None:
                        self._window.select_edge(e, additive=bool(add))
                        self._drag_mode = "edge"
                        self._last_world = self._window.screen_to_world(
                            self._press_pos
                        )
                        return
                else:
                    if self._hit_face_handle(self._press_pos):
                        self._window.select_face(True)
                        self._drag_mode = "face"
                        self._last_world = self._window.screen_to_world(
                            self._press_pos
                        )
                        return
            elif obj is not None and self._window.transform_mode == "rect":
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
            elif obj is not None and not self._window.modeling and self._window.transform_mode == "move":
                handle = self._hit_move_handle(self._press_pos)
                if handle:
                    self._drag_mode = handle
                    self._last_world = self._window.screen_to_world(self._press_pos)
            elif obj is not None and not self._window.modeling and self._window.transform_mode == "scale":
                handle = self._hit_scale_handle(self._press_pos)
                if handle:
                    self._drag_mode = handle
                    self._last_world = self._window.screen_to_world(self._press_pos)
            elif obj is not None and not self._window.modeling and self._window.transform_mode == "rotate":
                if self._hit_rotate_handle(self._press_pos):
                    self._drag_mode = "rotate"
                    self._last_world = self._window.screen_to_world(self._press_pos)
            if self._drag_mode == "pan" and self._press_hit is None:
                self._window.select_object(None)
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
                if (
                    self._drag_mode == "vertex"
                    and self._last_world is not None
                    and self._drag_vertex is not None
                ):
                    obj = self._window.selected_obj
                    if obj is not None and getattr(obj, "mesh", None) is not None:
                        dx = wx - self._last_world[0]
                        dy = wy - self._last_world[1]
                        vx, vy = obj.mesh.vertices[self._drag_vertex]
                        wx0, wy0 = self._window.mesh_to_world(obj, vx, vy)
                        new_x = wx0 + dx
                        new_y = wy0 + dy
                        obj.mesh.vertices[self._drag_vertex] = self._window.world_to_mesh(obj, new_x, new_y)
                        self._window.draw_scene(update_list=False)
                    self._last_world = (wx, wy)
                elif (
                    self._drag_mode in {"edge", "face"}
                    and self._last_world is not None
                ):
                    dx = wx - self._last_world[0]
                    dy = wy - self._last_world[1]
                    self._window.translate_selection(dx, dy)
                    self._last_world = (wx, wy)
                elif self._drag_mode in ("move", "move_x", "move_y", "move_xy") and self._last_world is not None:
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
                        if self._window.snap_to_grid:
                            obj.x = self._snap(obj.x, self._window.move_step)
                            obj.y = self._snap(obj.y, self._window.move_step)
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
                            if self._window.snap_to_grid:
                                obj.scale_x = self._snap(obj.scale_x, self._window.scale_step)
                                obj.scale_y = self._snap(obj.scale_y, self._window.scale_step)
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
                            if self._window.snap_to_grid:
                                obj.scale_x = self._snap(obj.scale_x, self._window.scale_step)
                                obj.scale_y = self._snap(obj.scale_y, self._window.scale_step)
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
                        if self._window.snap_to_grid:
                            obj.angle = self._snap(obj.angle, self._window.rotate_step)
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
            if self._press_pos is not None and not self._dragging and self._drag_mode == "pan":
                if self._press_hit is None:
                    self._window.select_object(None)
                else:
                    wx, wy = self._window.screen_to_world(pos)
                    obj = self._window.find_object_at(wx, wy)
                    mods = ev.modifiers() if hasattr(ev, "modifiers") else 0
                    add = mods & (
                        Qt.KeyboardModifier.ControlModifier
                        | Qt.KeyboardModifier.ShiftModifier
                    )
                    self._window.select_object(obj, additive=bool(add))
            self._press_pos = None
            self._dragging = False
            self._last_pos = None
            self._drag_mode = None
            self._drag_corner = None
            self._drag_vertex = None
            self._last_world = None
            self._drag_angle = None
            self._press_hit = None
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
            if self._window.modeling and getattr(obj, "mesh", None) is not None:
                extr_a = menu.addAction("Extrude") if hasattr(menu, "addAction") else None
                face_a = menu.addAction("New Face") if hasattr(menu, "addAction") else None
                loop_a = menu.addAction("Loop Cut") if hasattr(menu, "addAction") else None
                fill_a = menu.addAction("Toggle Fill") if hasattr(menu, "addAction") else None
                del_a = menu.addAction("Delete") if hasattr(menu, "addAction") else None
                if extr_a is not None and hasattr(extr_a, "triggered"):
                    extr_a.triggered.connect(self._window.extrude_selection)
                if face_a is not None and hasattr(face_a, "triggered"):
                    face_a.triggered.connect(self._window.new_face_from_edge)
                if loop_a is not None and hasattr(loop_a, "triggered"):
                    loop_a.triggered.connect(self._window.loop_cut)
                if fill_a is not None and hasattr(fill_a, "triggered"):
                    fill_a.triggered.connect(self._window.toggle_fill)
                if del_a is not None and hasattr(del_a, "triggered"):
                    del_a.triggered.connect(self._window.delete_selected)
            else:
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
            self._window.select_object(None)
            sprite_a = menu.addAction("Create Sprite") if hasattr(menu, "addAction") else None
            empty_a = menu.addAction("Create Empty") if hasattr(menu, "addAction") else None
            cam_a = menu.addAction("Create Camera") if hasattr(menu, "addAction") else None
            shape_m = menu.addMenu("Create Shape") if hasattr(menu, "addMenu") else None

            if sprite_a is not None and hasattr(sprite_a, "triggered"):
                sprite_a.triggered.connect(lambda: self._window.create_object(wx, wy))
            if empty_a is not None and hasattr(empty_a, "triggered"):
                empty_a.triggered.connect(lambda: self._window.create_empty(wx, wy))
            if cam_a is not None and hasattr(cam_a, "triggered"):
                cam_a.triggered.connect(lambda: self._window.create_camera(wx, wy))
            if shape_m is not None:
                sq_a = shape_m.addAction("Square") if hasattr(shape_m, "addAction") else None
                tri_a = shape_m.addAction("Triangle") if hasattr(shape_m, "addAction") else None
                cir_a = shape_m.addAction("Circle") if hasattr(shape_m, "addAction") else None
                poly_a = shape_m.addAction("Polygon") if hasattr(shape_m, "addAction") else None
                if sq_a is not None and hasattr(sq_a, "triggered"):
                    sq_a.triggered.connect(lambda: self._window.create_shape("square", wx, wy))
                if tri_a is not None and hasattr(tri_a, "triggered"):
                    tri_a.triggered.connect(lambda: self._window.create_shape("triangle", wx, wy))
                if cir_a is not None and hasattr(cir_a, "triggered"):
                    cir_a.triggered.connect(lambda: self._window.create_shape("circle", wx, wy))
                if poly_a is not None and hasattr(poly_a, "triggered"):
                    poly_a.triggered.connect(lambda: self._window.create_shape("polygon", wx, wy))
        menu.exec(cast(QWidget, self).mapToGlobal(point))





