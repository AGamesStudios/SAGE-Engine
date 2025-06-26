from PyQt6.QtWidgets import QWidget
from PyQt6.QtCore import QTimer, Qt, QPointF

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
        self._drag_offset = (0.0, 0.0)
        self.selected_obj: GameObject | None = None

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

    def _tick(self) -> None:
        self.renderer.draw_scene(self.scene, self.camera, gizmos=True,
                                 selected=self.selected_obj)

    def showEvent(self, event):  # pragma: no cover
        self.timer.start()
        super().showEvent(event)

    def hideEvent(self, event):  # pragma: no cover
        self.timer.stop()
        super().hideEvent(event)

    # mouse drag for panning -------------------------------------------------

    def mousePressEvent(self, event):  # pragma: no cover - UI interaction
        if event.buttons() & Qt.MouseButton.LeftButton:
            hit = self._hit_gizmo(event.position())
            if hit:
                self._gizmo_drag = hit
                world = self._screen_to_world(event.position())
                if self.selected_obj:
                    self._drag_offset = (
                        self.selected_obj.x - world[0],
                        self.selected_obj.y - world[1],
                    )
            else:
                self._drag_pos = event.position()
            self.setCursor(Qt.CursorShape.BlankCursor)
            self.grabMouse()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):  # pragma: no cover - UI interaction
        if self._gizmo_drag and self.selected_obj is not None:
            world = self._screen_to_world(event.position())
            if 'x' in self._gizmo_drag:
                self.selected_obj.x = world[0] + self._drag_offset[0]
            if 'y' in self._gizmo_drag:
                self.selected_obj.y = world[1] + self._drag_offset[1]
            if self.editor:
                self.editor._update_transform_panel()
                self.editor._mark_dirty()
            self.update()
        elif self._drag_pos is not None and event.buttons() & Qt.MouseButton.LeftButton:
            dx = event.position().x() - self._drag_pos.x()
            dy = event.position().y() - self._drag_pos.y()
            scale = units.UNITS_PER_METER
            self.camera.x -= dx / scale
            self.camera.y += (1 if units.Y_UP else -1) * dy / scale
            self._drag_pos = event.position()
            self.update()
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):  # pragma: no cover - UI interaction
        self._drag_pos = None
        self._gizmo_drag = None
        self.releaseMouse()
        self.setCursor(Qt.CursorShape.ArrowCursor)
        super().mouseReleaseEvent(event)

    def wheelEvent(self, event):  # pragma: no cover - zoom control
        delta = event.angleDelta().y() / 120
        if delta:
            pos = event.position()
            before = self._screen_to_world(pos)
            self.camera.zoom *= 1.0 + (0.1 * delta)
            self.camera.zoom = max(0.1, min(10.0, self.camera.zoom))
            if self._gizmo_drag:
                after = self._screen_to_world(pos)
                self._drag_offset = (
                    self._drag_offset[0] + before[0] - after[0],
                    self._drag_offset[1] + before[1] - after[1],
                )
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

    def _hit_gizmo(self, pos: QPointF) -> str | None:
        if self.selected_obj is None:
            return None
        sx, sy = self._world_to_screen(self.selected_obj.x, self.selected_obj.y)
        dx = pos.x() - sx
        dy = pos.y() - sy
        # Use world unit scale so hit testing matches the drawn gizmo
        scale = units.UNITS_PER_METER
        arrow = 50.0 * scale
        handle = 8.0 * scale
        sign = -1.0 if units.Y_UP else 1.0
        if abs(dx) < handle and abs(dy) < handle:
            return 'xy'
        if abs(dy) < handle and 0 <= dx <= arrow:
            return 'x'
        if abs(dx) < handle and 0 <= sign*dy <= arrow:
            return 'y'
        return None
