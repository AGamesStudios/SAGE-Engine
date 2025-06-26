from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QColor, QPixmap
from PyQt6.QtCore import QRectF

from ..core.camera import Camera
from .. import units


class PainterWidget(QWidget):
    """Simple QWidget that delegates painting to its renderer."""

    def __init__(self, parent=None):
        super().__init__(parent)
        # painter renderer instance will be set by the parent
        self.renderer = None

    def paintEvent(self, event):  # pragma: no cover - GUI callback
        if self.renderer:
            self.renderer.paint()


@dataclass
class QtPainterRenderer:
    """Renderer using :class:`QPainter` for basic 2D drawing."""

    width: int = 640
    height: int = 480
    title: str = "SAGE 2D"
    widget: Optional[PainterWidget] = None

    def create_widget(self) -> PainterWidget:
        return PainterWidget()

    def __post_init__(self):
        if self.widget is None:
            self.widget = self.create_widget()
        self.widget.renderer = self
        self.widget.resize(self.width, self.height)
        self._should_close = False
        self._scene = None
        self._camera = None
        # cache loaded pixmaps by image id
        self._pixmaps = {}

    def should_close(self) -> bool:
        return self._should_close

    def set_window_size(self, width: int, height: int) -> None:
        if self.widget:
            self.widget.resize(width, height)
        self.width = width
        self.height = height

    def clear(self, color=(0, 0, 0)) -> None:
        painter = QPainter(self.widget)
        painter.fillRect(self.widget.rect(), QColor(*color))
        painter.end()

    def draw_scene(self, scene, camera: Camera | None = None) -> None:
        self._scene = scene
        self._camera = camera
        self.widget.update()

    def paint(self) -> None:
        if self._scene is None:
            return
        painter = QPainter(self.widget)
        painter.fillRect(self.widget.rect(), QColor(0, 0, 0))
        self._render_scene(painter, self._scene, self._camera)
        painter.end()

    # internal helpers -----------------------------------------------------
    def _get_pixmap(self, obj) -> QPixmap:
        if obj.image is None:
            pm = QPixmap(obj.width, obj.height)
            pm.fill(QColor(*(obj.color or (255, 255, 255, 255))))
            return pm
        key = id(obj.image)
        pm = self._pixmaps.get(key)
        if pm is not None:
            return pm
        from PIL.ImageQt import ImageQt
        pm = QPixmap.fromImage(ImageQt(obj.image))
        self._pixmaps[key] = pm
        return pm

    def _render_scene(self, painter: QPainter, scene, camera: Camera | None) -> None:
        scale = units.UNITS_PER_METER
        sign = 1.0 if units.Y_UP else -1.0
        painter.save()
        painter.translate(self.width / 2, self.height / 2)
        if units.Y_UP:
            painter.scale(1, -1)
        if camera:
            painter.scale(camera.zoom, camera.zoom)
            painter.translate(-camera.x * scale, -camera.y * scale * sign)
        scene.sort_objects()
        for obj in scene.objects:
            if isinstance(obj, Camera):
                continue
            pm = self._get_pixmap(obj)
            painter.save()
            painter.translate(obj.x * scale, obj.y * scale * sign)
            painter.rotate(obj.angle)
            painter.scale(obj.scale_x * scale, obj.scale_y * scale)
            x = -obj.width * obj.pivot_x
            y = -obj.height * obj.pivot_y
            target = QRectF(x, y, obj.width, obj.height)
            painter.drawPixmap(target, pm, pm.rect())
            painter.restore()
        # draw origin cross for orientation
        painter.setPen(QColor(255, 0, 0))
        painter.drawLine(-50 * scale, 0, 50 * scale, 0)
        painter.setPen(QColor(0, 255, 0))
        painter.drawLine(0, -50 * scale, 0, 50 * scale)
        painter.restore()

    def present(self) -> None:
        self.widget.update()

    def close(self) -> None:
        self._should_close = True
        if self.widget:
            self.widget.close()
