from PyQt6.QtWidgets import QWidget, QToolButton, QFrame, QVBoxLayout
from PyQt6.QtCore import QTimer, Qt, QPointF
import math
        self._gizmo_mode = 'move'
        # toolbar for selecting gizmo mode
        self.bar = QFrame(self)
        layout = QVBoxLayout(self.bar)
        layout.setContentsMargins(2, 2, 2, 2)
        layout.setSpacing(2)
        self.move_btn = QToolButton(self.bar)
        self.move_btn.setText('M')
        self.move_btn.setCheckable(True)
        self.move_btn.setChecked(True)
        self.move_btn.clicked.connect(lambda: self.set_gizmo_mode('move'))
        if self.editor:
            self.move_btn.setToolTip(self.editor.t('mode_move'))
        layout.addWidget(self.move_btn)
        self.rot_btn = QToolButton(self.bar)
        self.rot_btn.setText('R')
        self.rot_btn.setCheckable(True)
        self.rot_btn.clicked.connect(lambda: self.set_gizmo_mode('rotate'))
        if self.editor:
            self.rot_btn.setToolTip(self.editor.t('mode_rotate'))
        layout.addWidget(self.rot_btn)
        self.scale_btn = QToolButton(self.bar)
        self.scale_btn.setText('S')
        self.scale_btn.setCheckable(True)
        self.scale_btn.clicked.connect(lambda: self.set_gizmo_mode('scale'))
        if self.editor:
            self.scale_btn.setToolTip(self.editor.t('mode_scale'))
        layout.addWidget(self.scale_btn)
        self.bar.setFrameShape(QFrame.Shape.StyledPanel)
        self.bar.show()
        self.bar.move(8, 8)

        self.bar.move(8, 8)
            mode=self._gizmo_mode,
import math

from engine.renderers.opengl_renderer import OpenGLRenderer, GLWidget
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
        self.renderer = OpenGLRenderer(self.width(), self.height(), widget=self)
        self.timer = QTimer(self)
        self.timer.setInterval(33)  # ~30 FPS to reduce CPU load
        self.timer.timeout.connect(self._tick)
        self.timer.start()
        self.setMinimumSize(200, 150)
        self._drag_pos = None
        self._gizmo_drag = None
        self._gizmo_hover = None
        self._drag_offset = (0.0, 0.0)
        self.selected_obj: GameObject | None = None
        self._cursor_world: tuple[float, float] | None = None

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
        )

                    if hit == 'rot':
                        ang = math.degrees(math.atan2(
                            world[1] - self.selected_obj.y,
                            world[0] - self.selected_obj.x,
                        ))
                        self._drag_offset = self.selected_obj.angle - ang
                    elif hit == 'sx':
                        self._drag_offset = (
                            world[0] - self.selected_obj.x,
                            self.selected_obj.scale_x,
                        )
                    elif hit == 'sy':
                        self._drag_offset = (
                            world[1] - self.selected_obj.y,
                            self.selected_obj.scale_y,
                        )
                    else:
                        self._drag_offset = (
                            self.selected_obj.x - world[0],
                            self.selected_obj.y - world[1],
                        )
            if self._gizmo_drag == 'rot':
                ang = math.degrees(math.atan2(
                    world[1] - self.selected_obj.y,
                    world[0] - self.selected_obj.x,
                ))
                self.selected_obj.angle = ang + self._drag_offset
            elif self._gizmo_drag == 'sx':
                start_dx, base = self._drag_offset
                dx = world[0] - self.selected_obj.x
                if start_dx:
                    self.selected_obj.scale_x = max(0.01, base * (dx / start_dx))
            elif self._gizmo_drag == 'sy':
                start_dy, base = self._drag_offset
                dy = world[1] - self.selected_obj.y
                if start_dy:
                    self.selected_obj.scale_y = max(0.01, base * (dy / start_dy))
            else:
                if 'x' in self._gizmo_drag:
                    self.selected_obj.x = world[0] + self._drag_offset[0]
                if 'y' in self._gizmo_drag:
                    self.selected_obj.y = world[1] + self._drag_offset[1]
            if self._gizmo_drag in ('x', 'y', 'xy'):
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
                    if hit == 'rot':
                        ang = math.degrees(math.atan2(
                            world[1] - self.selected_obj.y,
                            world[0] - self.selected_obj.x,
                        ))
                        self._drag_offset = self.selected_obj.angle - ang
                    elif hit == 'sx':
                        self._drag_offset = (
                            world[0] - self.selected_obj.x,
                            self.selected_obj.scale_x,
                        )
                    elif hit == 'sy':
                        self._drag_offset = (
                            world[1] - self.selected_obj.y,
                            self.selected_obj.scale_y,
                        )
                    else:
                        self._drag_offset = (
                            self.selected_obj.x - world[0],
                            self.selected_obj.y - world[1],
                        )
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
                ang = math.degrees(math.atan2(
                    world[1] - self.selected_obj.y,
                    world[0] - self.selected_obj.x,
                ))
                self.selected_obj.angle = ang + self._drag_offset
            elif self._gizmo_drag == 'sx':
                start_dx, base = self._drag_offset
                dx = world[0] - self.selected_obj.x
                if start_dx:
    def set_gizmo_mode(self, mode: str) -> None:
        """Switch the transform gizmo mode."""
        self._gizmo_mode = mode
        self.move_btn.setChecked(mode == 'move')
        self.rot_btn.setChecked(mode == 'rotate')
        self.scale_btn.setChecked(mode == 'scale')
        self.update()

                    self.selected_obj.scale_x = max(0.01, base * (dx / start_dx))
            elif self._gizmo_drag == 'sy':
                start_dy, base = self._drag_offset
                dy = world[1] - self.selected_obj.y
                if start_dy:
                    self.selected_obj.scale_y = max(0.01, base * (dy / start_dy))
            else:
                if 'x' in self._gizmo_drag:
                    self.selected_obj.x = world[0] + self._drag_offset[0]
                if 'y' in self._gizmo_drag:
                    self.selected_obj.y = world[1] + self._drag_offset[1]
            if self.editor:
                self.editor._update_transform_panel()
                self.editor._mark_dirty()
            self.update()
            self._cursor_world = world
        elif self._drag_pos is not None and event.buttons() & Qt.MouseButton.LeftButton:
            dx = event.position().x() - self._drag_pos.x()
            dy = event.position().y() - self._drag_pos.y()
            scale = units.UNITS_PER_METER
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
        self._gizmo_drag = None
        self.releaseMouse()
        self.setCursor(Qt.CursorShape.ArrowCursor)
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
                self._drag_offset = (
                    self._drag_offset[0] + before[0] - after[0],
                    self._drag_offset[1] + before[1] - after[1],
        ring = arrow * 1.2
        ring_tol = 6.0 * ratio
        if abs(dx - arrow) <= handle and abs(dy) <= handle:
            return 'sx'
        if abs(dx) <= handle and abs(sign*dy - arrow) <= handle:
        if self._gizmo_mode == 'move':
            if abs(dy) < handle and 0 <= dx <= arrow:
                return 'x'
            if abs(dx) < handle and 0 <= sign*dy <= arrow:
                return 'y'
        elif self._gizmo_mode == 'scale':
            if abs(dx - arrow) <= handle and abs(dy) <= handle:
                return 'sx'
            if abs(dx) <= handle and abs(sign*dy - arrow) <= handle:
                return 'sy'
        elif self._gizmo_mode == 'rotate':
            dist = (dx**2 + dy**2) ** 0.5
            if ring - ring_tol <= dist <= ring + ring_tol:
                return 'rot'
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

    def _hit_gizmo(self, pos: QPointF) -> str | None:
        if self.selected_obj is None:
            return None
        sx, sy = self._world_to_screen(self.selected_obj.x, self.selected_obj.y)
        dx = pos.x() - sx
        dy = pos.y() - sy
        # Use world unit scale so hit testing matches the drawn gizmo
        ratio = self.devicePixelRatioF()
        arrow = 50.0 * ratio
        handle = 8.0 * ratio
        ring = arrow * 1.2
        ring_tol = 6.0 * ratio
        sign = -1.0 if units.Y_UP else 1.0
        if abs(dx) < handle and abs(dy) < handle:
            return 'xy'
        if abs(dy) < handle and 0 <= dx <= arrow:
            return 'x'
        if abs(dx) < handle and 0 <= sign*dy <= arrow:
            return 'y'
        if abs(dx - arrow) <= handle and abs(dy) <= handle:
            return 'sx'
        if abs(dx) <= handle and abs(sign*dy - arrow) <= handle:
            return 'sy'
        dist = (dx**2 + dy**2) ** 0.5
        if ring - ring_tol <= dist <= ring + ring_tol:
            return 'rot'
        return None
