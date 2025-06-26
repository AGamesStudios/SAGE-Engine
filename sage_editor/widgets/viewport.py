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
            self.camera.zoom *= 1.0 + (0.1 * delta)
            self.camera.zoom = max(0.1, min(10.0, self.camera.zoom))
            self.update()
        super().wheelEvent(event)

    # ------------------------------------------------------------------
    def set_selected(self, obj: GameObject | None) -> None:
        self.selected_obj = obj
        self.update()

    # coordinate helpers ------------------------------------------------
    def _world_to_screen(self, x: float, y: float) -> tuple[float, float]:
        scale = units.UNITS_PER_METER * self.camera.zoom
        sx = (x - self.camera.x) * scale + self.width() / 2
        sign = 1.0 if units.Y_UP else -1.0
        sy = self.height() / 2 - (y - self.camera.y) * scale * sign
        return sx, sy

    def _screen_to_world(self, pos: QPointF) -> tuple[float, float]:
        scale = units.UNITS_PER_METER * self.camera.zoom
        sign = 1.0 if units.Y_UP else -1.0
        x = self.camera.x + (pos.x() - self.width() / 2) / scale
        y = self.camera.y + sign * (self.height() / 2 - pos.y()) / scale
        return x, y

    def _hit_gizmo(self, pos: QPointF) -> str | None:
        if self.selected_obj is None:
            return None
        sx, sy = self._world_to_screen(self.selected_obj.x, self.selected_obj.y)
        dx = pos.x() - sx
        dy = pos.y() - sy
        arrow = 50.0
        handle = 8.0
        sign = -1.0 if units.Y_UP else 1.0
        if abs(dx) < handle and abs(dy) < handle:
            return 'xy'
        if abs(dy) < handle and 0 <= dx <= arrow:
            return 'x'
        if abs(dx) < handle and 0 <= sign*dy <= arrow:
            return 'y'
        return None
