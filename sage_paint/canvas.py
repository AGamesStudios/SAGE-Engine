from __future__ import annotations

from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QImage, QColor
from PyQt6.QtCore import Qt, QPoint, QPointF

from .tools import BrushTool, EraserTool, Tool


class Canvas(QWidget):
    """Widget providing a zoomable, pannable painting surface."""

    def __init__(self, width: int = 512, height: int = 512, parent: QWidget | None = None):
        super().__init__(parent)
        self.image = QImage(width, height, QImage.Format.Format_RGB32)
        self.image.fill(QColor('white'))
        self.zoom_level = 1.0
        self.offset = QPointF(0, 0)
        self.pen_color = QColor('black')
        self.pen_width = 2
        self._tool: Tool = BrushTool(self)
        self._panning = False
        self._pan_start = QPoint()
        self.setMouseTracking(True)

    def zoom(self, factor: float) -> None:
        """Scale the view by ``factor`` and refresh."""
        self.zoom_level *= factor
        self.update()

    # Tool management ----------------------------------------------------

    def set_tool(self, tool: Tool) -> None:
        self._tool = tool
        self.update()

    def use_brush(self) -> None:
        self.set_tool(BrushTool(self))

    def use_eraser(self) -> None:
        self.set_tool(EraserTool(self))

    # Coordinate helpers -------------------------------------------------

    def view_to_image(self, point: QPointF) -> QPoint:
        x = (point.x() - self.offset.x()) / self.zoom_level
        y = (point.y() - self.offset.y()) / self.zoom_level
        return QPoint(int(x), int(y))

    # Painting -----------------------------------------------------------

    def paintEvent(self, event) -> None:  # pragma: no cover - Qt paint
        painter = QPainter(self)
        painter.fillRect(self.rect(), Qt.GlobalColor.white)
        painter.translate(self.offset)
        painter.scale(self.zoom_level, self.zoom_level)
        painter.drawImage(0, 0, self.image)
        painter.end()

    def mousePressEvent(self, event) -> None:
        if event.button() == Qt.MouseButton.LeftButton:
            img_pos = self.view_to_image(event.position())
            self._tool.press(img_pos)
        elif event.button() == Qt.MouseButton.MiddleButton:
            self._panning = True
            self._pan_start = event.pos()

    def mouseMoveEvent(self, event) -> None:
        if self._panning:
            delta = event.pos() - self._pan_start
            self.offset += delta
            self._pan_start = event.pos()
            self.update()
        else:
            if event.buttons() & Qt.MouseButton.LeftButton:
                img_pos = self.view_to_image(event.position())
                self._tool.move(img_pos)
                self.update()

    def mouseReleaseEvent(self, event) -> None:
        if event.button() == Qt.MouseButton.LeftButton:
            img_pos = self.view_to_image(event.position())
            self._tool.release(img_pos)
        elif event.button() == Qt.MouseButton.MiddleButton:
            self._panning = False

    def wheelEvent(self, event) -> None:
        if event.angleDelta().y() > 0:
            factor = 1.2
        else:
            factor = 1 / 1.2
        self.zoom(factor)
