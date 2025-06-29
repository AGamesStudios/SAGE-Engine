from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QToolButton, QButtonGroup
)
from PyQt6.QtGui import QIcon, QKeySequence, QShortcut
from PyQt6.QtCore import QTimer, Qt, QPointF
import math

from engine.renderers.opengl_renderer import OpenGLRenderer, GLWidget
from ..icons import load_icon
from engine.core.scene import Scene
from engine.core.camera import Camera
from engine.core.game_object import GameObject
from engine import units


class Viewport(GLWidget):
    """Viewport widget that renders the current scene using OpenGL."""

    def __init__(self, scene: Scene, editor=None, parent=None):
        super().__init__(parent)
        self.scene = scene
        self.editor = editor
        if not scene.objects:
            square = GameObject(color=(0, 255, 0, 255))
            scene.add_object(square)
        cam = scene.get_active_camera()
        if cam is not None:
            self.camera = Camera(
                x=cam.x,
                y=cam.y,
                width=cam.width,
                height=cam.height,
                zoom=cam.zoom,
            )
        else:
            self.camera = Camera(
                x=0,
                y=0,
                width=self.width(),
                height=self.height(),
            )
        self._center_camera()
        # default rendering options
        self.show_grid = False
        self.show_axes = True
        self.snap_to_grid = False
        self.grid_size = 1.0
        self.renderer = OpenGLRenderer(self.width(), self.height(), widget=self)
        self.renderer.show_grid = self.show_grid
        self.renderer.show_axes = self.show_axes
        # disable sprite effects inside the editor so objects stay put
        self.renderer.apply_effects = False
        self.renderer.grid_size = self.grid_size
        self.timer = QTimer(self)
        self.timer.setInterval(33)  # ~30 FPS to reduce CPU load
        self.timer.timeout.connect(self._tick)
        self.timer.start()
        self.setMinimumSize(200, 150)
        self._drag_pos = None
        self._gizmo_drag = None
        self._gizmo_hover = None
        self._drag_offset = (0.0, 0.0)
        self._drag_start_world = (0.0, 0.0)
        self._drag_start_obj = (0.0, 0.0)
        self._drag_start_angle = 0.0
        self._drag_prev_state = None
        self.selected_obj: GameObject | None = None
        self._cursor_world: tuple[float, float] | None = None

        # active transform mode: 'pan', 'move', 'rotate', or 'scale'
        self._transform_mode = 'pan'
        # use local or world coordinates for gizmo orientation
        self._local_coords = False

        # small toolbar with transform mode buttons
        self.transform_bar = QWidget(self)
        layout = QVBoxLayout(self.transform_bar)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(2)

        modes = [
            ('pan', 'hand.png'),
            ('move', 'move.png'),
            ('rotate', 'rotate.png'),
            ('scale', 'scale.png'),
        ]
        self.transform_buttons: list[QToolButton] = []
        self.button_group = QButtonGroup(self.transform_bar)
        self.button_group.setExclusive(True)
        for i, (mode, icon) in enumerate(modes):
            btn = QToolButton(self.transform_bar)
            btn.setFixedSize(24, 24)
            btn.setIcon(load_icon(icon))
            btn.setCheckable(True)
            if i == 0:
                btn.setChecked(True)
            btn.clicked.connect(lambda _, m=mode: self.set_transform_mode(m))
            layout.addWidget(btn)
            self.button_group.addButton(btn)
            self.transform_buttons.append(btn)
        self.transform_bar.move(4, 4)
        self.transform_bar.show()
        self.transform_bar.raise_()

        # keyboard shortcuts for viewport controls
        self.shortcut_focus = QShortcut(QKeySequence('F'), self)
        self.shortcut_focus.activated.connect(self._focus_selected)
        self.shortcut_pan = QShortcut(QKeySequence('Q'), self)
        self.shortcut_pan.activated.connect(lambda: self.set_transform_mode('pan'))
        self.shortcut_move = QShortcut(QKeySequence('W'), self)
        self.shortcut_move.activated.connect(lambda: self.set_transform_mode('move'))
        self.shortcut_rot = QShortcut(QKeySequence('E'), self)
        self.shortcut_rot.activated.connect(lambda: self.set_transform_mode('rotate'))
        self.shortcut_scale = QShortcut(QKeySequence('R'), self)
        self.shortcut_scale.activated.connect(lambda: self.set_transform_mode('scale'))

    def set_coord_mode(self, local: bool) -> None:
        """Set whether gizmos follow the object's rotation."""
        self._local_coords = bool(local)

    def set_show_grid(self, enabled: bool) -> None:
        """Toggle the background grid."""
        self.show_grid = bool(enabled)
        if self.renderer:
            self.renderer.show_grid = self.show_grid
        self.update()

    def set_show_axes(self, enabled: bool) -> None:
        """Toggle the coordinate axes gizmo."""
        self.show_axes = bool(enabled)
        if self.renderer:
            self.renderer.show_axes = self.show_axes
        self.update()

    def set_snap(self, enabled: bool) -> None:
        """Enable or disable snapping to the grid."""
        self.snap_to_grid = bool(enabled)

    def set_grid_size(self, size: float) -> None:
        """Set the spacing of the background grid and snapping."""
        if size <= 0:
            return
        self.grid_size = float(size)
        if self.renderer:
            self.renderer.grid_size = self.grid_size
        self.update()

    def set_grid_color(self, color: tuple[int, int, int]) -> None:
        """Set the display color of the grid."""
        if self.renderer:
            self.renderer.grid_color = (
                color[0] / 255.0,
                color[1] / 255.0,
                color[2] / 255.0,
                1.0,
            )
        self.update()

    def set_transform_mode(self, mode: str) -> None:
        """Set the active transform interaction mode."""
        if mode not in {'pan', 'move', 'rotate', 'scale'}:
            return
        self._transform_mode = mode
        # ensure only the matching button is checked
        idx = {'pan': 0, 'move': 1, 'rotate': 2, 'scale': 3}[mode]
        if 0 <= idx < len(self.transform_buttons):
            self.transform_buttons[idx].setChecked(True)

    def _center_camera(self) -> None:
        """Center the camera on existing objects."""
        objs = [o for o in self.scene.objects if not isinstance(o, Camera)]
        if not objs:
            self.camera.x = 0
            self.camera.y = 0
            return
        xs = [o.x for o in objs]
        ys = [o.y for o in objs]
        self.camera.x = sum(xs) / len(xs)
        self.camera.y = sum(ys) / len(ys)

    def _focus_selected(self) -> None:
        """Center the viewport camera on the selected object."""
        if self.selected_obj is None:
            return
        self.camera.x = self.selected_obj.x
        self.camera.y = self.selected_obj.y
        self.update()

    # QWidget overrides -------------------------------------------------------

    def resizeEvent(self, event):  # pragma: no cover - handle resize
        super().resizeEvent(event)
        self.renderer.set_window_size(self.width(), self.height())

    def closeEvent(self, event):  # pragma: no cover - cleanup
        self.timer.stop()
        self.renderer.close()
        super().closeEvent(event)

    def set_scene(self, scene: Scene) -> None:
        self.scene = scene
        if not scene.objects:
            square = GameObject(color=(0, 255, 0, 255))
            scene.add_object(square)
        cam = scene.get_active_camera()
        if cam is not None:
            self.camera = Camera(
                x=cam.x,
                y=cam.y,
                width=cam.width,
                height=cam.height,
                zoom=cam.zoom,
            )
        else:
            self.camera = Camera(
                x=0,
                y=0,
                width=self.width(),
                height=self.height(),
            )
        self._center_camera()
        self._cursor_world = None

    def _tick(self) -> None:
        self.renderer.draw_scene(
            self.scene,
            self.camera,
            gizmos=True,
            selected=self.selected_obj,
            hover=self._gizmo_hover,
            dragging=self._gizmo_drag,
            cursor=self._cursor_world,
            mode=self._transform_mode,
            local=self._local_coords,
        )

    def showEvent(self, event):  # pragma: no cover
        self.timer.start()
        super().showEvent(event)

    def hideEvent(self, event):  # pragma: no cover
        self.timer.stop()
        super().hideEvent(event)

    def leaveEvent(self, event):  # pragma: no cover - cursor left widget
        self._gizmo_hover = None
        self._cursor_world = None
        self.update()
        super().leaveEvent(event)

    # mouse drag for panning -------------------------------------------------

    def mousePressEvent(self, event):  # pragma: no cover - UI interaction
        if event.buttons() & Qt.MouseButton.LeftButton:
            hit = self._hit_gizmo(event.position())
            if hit:
                self._gizmo_drag = hit
                self._gizmo_hover = hit
                world = self._screen_to_world(event.position())
                self._cursor_world = world
                if self.selected_obj:
                    if self.editor:
                        self._drag_prev_state = self.editor._capture_state(self.selected_obj)
                    if hit == 'rot':
                        ang = math.degrees(math.atan2(
                            world[1] - self.selected_obj.y,
                            world[0] - self.selected_obj.x,
                        ))
                        self._drag_start_angle = ang
                        self._drag_offset = getattr(self.selected_obj, 'angle', 0.0)
                    elif hit == 'sx':
                        ang = math.radians(getattr(self.selected_obj, 'angle', 0.0))
                        cos_a = math.cos(ang)
                        sin_a = math.sin(ang)
                        start = (world[0] - self.selected_obj.x) * cos_a + (
                            world[1] - self.selected_obj.y
                        ) * sin_a
                        self._drag_offset = (start, getattr(self.selected_obj, 'scale_x', 1.0))
                    elif hit == 'sy':
                        ang = math.radians(getattr(self.selected_obj, 'angle', 0.0))
                        cos_a = math.cos(ang)
                        sin_a = math.sin(ang)
                        start = (world[0] - self.selected_obj.x) * (-sin_a) + (
                            world[1] - self.selected_obj.y
                        ) * cos_a
                        self._drag_offset = (start, getattr(self.selected_obj, 'scale_y', 1.0))
                    else:
                        self._drag_offset = (
                            self.selected_obj.x - world[0],
                            self.selected_obj.y - world[1],
                        )
                    self._drag_start_world = world
                    self._drag_start_obj = (self.selected_obj.x, self.selected_obj.y)
            else:
                obj = self._pick_object(event.position())
                if obj is not None:
                    self.selected_obj = obj
                    if self.editor:
                        for i, (_, o) in enumerate(self.editor.items):
                            if o is obj:
                                self.editor.object_combo.setCurrentIndex(i)
                                self.editor.object_list.setCurrentRow(i)
                                break
                    # automatically enable move mode when selecting via the hand tool
                    if self._transform_mode == 'pan':
                        self.set_transform_mode('move')
                    self._gizmo_hover = self._hit_gizmo(event.position())
                    self._cursor_world = self._screen_to_world(event.position())
                else:
                    self._drag_pos = event.position()
                    self._gizmo_hover = None
                    self._cursor_world = self._screen_to_world(event.position())
                    self.setCursor(Qt.CursorShape.BlankCursor)
                    self.grabMouse()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):  # pragma: no cover - UI interaction
        if self._gizmo_drag and self.selected_obj is not None:
            world = self._screen_to_world(event.position())
            if self._gizmo_drag == 'rot':
                ang = math.degrees(
                    math.atan2(
                        world[1] - self.selected_obj.y,
                        world[0] - self.selected_obj.x,
                    )
                )
                delta = ang - self._drag_start_angle
                if delta > 180:
                    delta -= 360
                elif delta < -180:
                    delta += 360
                self.selected_obj.angle = self._drag_offset + delta
            elif self._gizmo_drag == 'sx':
                start_dx, base = self._drag_offset
                ang = math.radians(getattr(self.selected_obj, 'angle', 0.0))
                cos_a = math.cos(ang)
                sin_a = math.sin(ang)
                dx_local = (world[0] - self.selected_obj.x) * cos_a + (
                    world[1] - self.selected_obj.y
                ) * sin_a
                if start_dx:
                    value = base * (dx_local / start_dx)
                    if self.snap_to_grid and self.grid_size > 0:
                        world_w = self.selected_obj.width * value
                        world_w = (
                            round(world_w / self.grid_size) * self.grid_size
                        )
                        value = max(0.01, world_w / self.selected_obj.width)
                    else:
                        value = max(0.01, value)
                    self.selected_obj.scale_x = value
            elif self._gizmo_drag == 'sy':
                start_dy, base = self._drag_offset
                ang = math.radians(getattr(self.selected_obj, 'angle', 0.0))
                cos_a = math.cos(ang)
                sin_a = math.sin(ang)
                dy_local = (world[0] - self.selected_obj.x) * (-sin_a) + (
                    world[1] - self.selected_obj.y
                ) * cos_a
                if start_dy:
                    value = base * (dy_local / start_dy)
                    if self.snap_to_grid and self.grid_size > 0:
                        world_h = self.selected_obj.height * value
                        world_h = (
                            round(world_h / self.grid_size) * self.grid_size
                        )
                        value = max(0.01, world_h / self.selected_obj.height)
                    else:
                        value = max(0.01, value)
                    self.selected_obj.scale_y = value
            else:
                dx = world[0] - self._drag_start_world[0]
                dy = world[1] - self._drag_start_world[1]
                if self._local_coords:
                    ang = math.radians(getattr(self.selected_obj, 'angle', 0.0))
                    cos_a = math.cos(ang)
                    sin_a = math.sin(ang)
                    proj_x = dx * cos_a + dy * sin_a
                    proj_y = dx * -sin_a + dy * cos_a
                    move_x = proj_x if 'x' in self._gizmo_drag else 0.0
                    move_y = proj_y if 'y' in self._gizmo_drag else 0.0
                    world_dx = move_x * cos_a - move_y * sin_a
                    world_dy = move_x * sin_a + move_y * cos_a
                else:
                    world_dx = dx if 'x' in self._gizmo_drag else 0.0
                    world_dy = dy if 'y' in self._gizmo_drag else 0.0
                new_x = self._drag_start_obj[0] + world_dx
                new_y = self._drag_start_obj[1] + world_dy
                if self.snap_to_grid and self.grid_size > 0:
                    new_x = round(new_x / self.grid_size) * self.grid_size
                    new_y = round(new_y / self.grid_size) * self.grid_size
                self.selected_obj.x = new_x
                self.selected_obj.y = new_y
            if self.editor:
                # update fields without rebuilding the variable panel
                self.editor._update_transform_panel(False)
                self.editor._mark_dirty()
            self.update()
            self._cursor_world = world
        elif self._drag_pos is not None and event.buttons() & Qt.MouseButton.LeftButton:
            dx = event.position().x() - self._drag_pos.x()
            dy = event.position().y() - self._drag_pos.y()
            scale = units.UNITS_PER_METER * self.camera.zoom
            self.camera.x -= dx / scale
            self.camera.y += (1 if units.Y_UP else -1) * dy / scale
            self._drag_pos = event.position()
            self.update()
            self._gizmo_hover = None
            self._cursor_world = self._screen_to_world(event.position())
        else:
            self._gizmo_hover = self._hit_gizmo(event.position())
        self._cursor_world = self._screen_to_world(event.position())
        self.update()
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):  # pragma: no cover - UI interaction
        self._drag_pos = None
        dragged = self._gizmo_drag
        self._gizmo_drag = None
        self.releaseMouse()
        self.setCursor(Qt.CursorShape.ArrowCursor)
        if dragged and self.editor and self.selected_obj and self._drag_prev_state:
            self.editor._record_undo(self.selected_obj, self._drag_prev_state)
        self._drag_prev_state = None
        self._gizmo_hover = self._hit_gizmo(event.position())
        self._cursor_world = self._screen_to_world(event.position())
        super().mouseReleaseEvent(event)

    def wheelEvent(self, event):  # pragma: no cover - zoom control
        delta = event.angleDelta().y() / 120
        if delta:
            pos = event.position()
            before = self._screen_to_world(pos)
            self.camera.zoom *= 1.0 + (0.1 * delta)
            self.camera.zoom = max(0.1, min(10.0, self.camera.zoom))
            if self._gizmo_drag in ('x', 'y', 'xy'):
                after = self._screen_to_world(pos)
                self._drag_start_world = (
                    self._drag_start_world[0] + before[0] - after[0],
                    self._drag_start_world[1] + before[1] - after[1],
                )
            self._gizmo_hover = self._hit_gizmo(pos)
            self._cursor_world = self._screen_to_world(pos)
            self.update()
        super().wheelEvent(event)

    # ------------------------------------------------------------------
    def set_selected(self, obj: GameObject | None) -> None:
        self.selected_obj = obj
        self.update()

    # coordinate helpers ------------------------------------------------
    def _viewport_rect(self) -> tuple[int, int, int, int]:
        """Return the active viewport in widget coordinates."""
        if not self.renderer.keep_aspect or self.camera is None:
            return 0, 0, self.width(), self.height()
        w = self.width()
        h = self.height()
        cam_ratio = self.camera.width / self.camera.height if self.camera.height else 1.0
        win_ratio = w / h if h else cam_ratio
        if cam_ratio > win_ratio:
            vp_w = w
            vp_h = int(w / cam_ratio)
            x = 0
            y = (h - vp_h) // 2
        else:
            vp_h = h
            vp_w = int(h * cam_ratio)
            x = (w - vp_w) // 2
            y = 0
        return x, y, vp_w, vp_h

    def _world_to_screen(self, x: float, y: float) -> tuple[float, float]:
        off_x, off_y, vp_w, vp_h = self._viewport_rect()
        scale = units.UNITS_PER_METER * self.camera.zoom
        sx = (x - self.camera.x) * scale + vp_w / 2 + off_x
        sign = 1.0 if units.Y_UP else -1.0
        sy = off_y + vp_h / 2 - (y - self.camera.y) * scale * sign
        return sx, sy

    def _screen_to_world(self, pos: QPointF) -> tuple[float, float]:
        off_x, off_y, vp_w, vp_h = self._viewport_rect()
        scale = units.UNITS_PER_METER * self.camera.zoom
        sign = 1.0 if units.Y_UP else -1.0
        sx = pos.x() - off_x
        sy = pos.y() - off_y
        x = self.camera.x + (sx - vp_w / 2) / scale
        y = self.camera.y + sign * (vp_h / 2 - sy) / scale
        return x, y

    def _pick_object(self, pos: QPointF) -> GameObject | None:
        """Return the topmost object under the cursor in world space."""
        world = self._screen_to_world(pos)
        self.scene.sort_objects()
        for obj in reversed(self.scene.objects):
            if isinstance(obj, Camera):
                icon_half = 16.0 / (obj.zoom if obj.zoom else 1.0)
                left = obj.x - icon_half
                bottom = obj.y - icon_half
                w = h = icon_half * 2
            else:
                left, bottom, w, h = obj.rect()
            if left <= world[0] <= left + w and bottom <= world[1] <= bottom + h:
                return obj
        return None

    def _hit_gizmo(self, pos: QPointF) -> str | None:
        if self.selected_obj is None or self._transform_mode == 'pan':
            return None
        sx, sy = self._world_to_screen(self.selected_obj.x, self.selected_obj.y)
        dx = pos.x() - sx
        dy = pos.y() - sy
        # Use world unit scale so hit testing matches the drawn gizmo
        ratio = self.devicePixelRatioF()
        arrow = 50.0 * ratio
        handle = 12.0 * ratio
        ring = arrow * 1.2
        ring_tol = 12.0 * ratio
        sign = -1.0 if units.Y_UP else 1.0
        # rotate into local coordinates when needed
        if self._transform_mode == 'scale' or self._local_coords:
            ang = math.radians(getattr(self.selected_obj, 'angle', 0.0))
            cos_a = math.cos(ang)
            sin_a = math.sin(ang)
            rdx = cos_a * dx - sin_a * dy
            rdy = sin_a * dx + cos_a * dy
            dx, dy = rdx, rdy
        result = None
        if abs(dx) < handle and abs(dy) < handle:
            result = 'xy'
        elif self._transform_mode == 'scale':
            if abs(dy) <= handle and 0 <= dx <= arrow + handle:
                result = 'sx'
            elif abs(dx) <= handle and 0 <= sign * dy <= arrow + handle:
                result = 'sy'
        elif abs(dy) <= handle and 0 <= dx <= arrow + handle:
            result = 'x'
        elif abs(dx) <= handle and 0 <= sign * dy <= arrow + handle:
            result = 'y'
        else:
            dist = (dx**2 + dy**2) ** 0.5
            if ring - ring_tol <= dist <= ring + ring_tol:
                result = 'rot'

        allowed = {'move': {'x', 'y', 'xy'}}
        if hasattr(self.selected_obj, 'angle'):
            allowed['rotate'] = {'rot'}
        if hasattr(self.selected_obj, 'scale_x') and hasattr(self.selected_obj, 'scale_y'):
            allowed['scale'] = {'sx', 'sy'}
        allowed = allowed.get(self._transform_mode, set())
        if result not in allowed:
            return None
        return result
