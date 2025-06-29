from __future__ import annotations

from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QImage, QColor, QPen
from PyQt6.QtCore import Qt, QPoint, QPointF

from .tools import BrushTool, EraserTool, FillTool, Tool


class Canvas(QWidget):
    """Widget providing a zoomable, pannable painting surface."""

    def __init__(self, width: int = 512, height: int = 512, parent: QWidget | None = None):
        super().__init__(parent)
        self.image = QImage(width, height, QImage.Format.Format_RGB32)
        self.image.fill(QColor('white'))
        self.zoom_level = 1.0
        self.offset = QPointF(0, 0)
        self.pen_color = QColor('black')
        self.brush_width = 2
        self.eraser_width = 8
        self.pen_width = self.brush_width
        self._tool: Tool = BrushTool(self)
        self.undo_stack: list[QImage] = []
        self.redo_stack: list[QImage] = []
        self._panning = False
        self._pan_start = QPointF()
        self._cursor = QPointF()
        self.setMouseTracking(True)
        self.smooth_pen = True

    def zoom(self, factor: float) -> None:
        """Scale the view by ``factor`` and refresh."""
        self.zoom_level *= factor
        self.update()

    def zoom_at(self, view_pos: QPointF, factor: float) -> None:
        """Zoom relative to ``view_pos`` keeping that point stable."""
        img_pos = self.view_to_image(view_pos)
        self.zoom_level *= factor
        self.offset = view_pos - QPointF(img_pos) * self.zoom_level
        self.update()

    # Tool management ----------------------------------------------------

    def set_tool(self, tool: Tool) -> None:
        self._tool = tool
        self.update()

    def use_brush(self) -> None:
        self.pen_width = self.brush_width
        self.set_tool(BrushTool(self))

    def use_eraser(self) -> None:
        self.pen_width = self.eraser_width
        self.set_tool(EraserTool(self))

    def use_fill(self) -> None:
        self.set_tool(FillTool(self))

    def set_brush_shape(self, shape: str) -> None:
        if isinstance(self._tool, BrushTool):
            self._tool.set_shape(shape)

    def set_pen_width(self, width: int) -> None:
        """Update pen width and remember size per tool."""
        if width < 1:
            width = 1
        self.pen_width = width
        if isinstance(self._tool, EraserTool):
            self.eraser_width = width
        else:
            self.brush_width = width

    # Undo/redo ---------------------------------------------------------

    def _push_undo(self) -> None:
        self.undo_stack.append(self.image.copy())
        self.redo_stack.clear()

    def undo(self) -> None:
        if not self.undo_stack:
            return
        self.redo_stack.append(self.image)
        self.image = self.undo_stack.pop()
        self.update()

    def redo(self) -> None:
        if not self.redo_stack:
            return
        self.undo_stack.append(self.image)
        self.image = self.redo_stack.pop()
        self.update()

    # Coordinate helpers -------------------------------------------------

    def view_to_image(self, point: QPointF) -> QPoint:
        x = (point.x() - self.offset.x()) / self.zoom_level
        y = (point.y() - self.offset.y()) / self.zoom_level
        return QPoint(int(x), int(y))

    # Painting -----------------------------------------------------------

    def paintEvent(self, event) -> None:  # pragma: no cover - Qt paint
        painter = QPainter(self)
        painter.fillRect(self.rect(), QColor(80, 80, 80))
        painter.translate(self.offset)
        painter.scale(self.zoom_level, self.zoom_level)
        painter.fillRect(0, 0, self.image.width(), self.image.height(), Qt.GlobalColor.white)
        painter.drawImage(0, 0, self.image)
        # canvas border
        pen = QPen(Qt.GlobalColor.black)
        pen.setWidth(1)
        painter.setPen(pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawRect(0, 0, self.image.width(), self.image.height())
        # draw current tool gizmo
        img_pos = self.view_to_image(self._cursor)
        self._tool.draw_gizmo(painter, img_pos)
        painter.end()

    def mousePressEvent(self, event) -> None:
        self._cursor = event.position()
        if event.button() == Qt.MouseButton.LeftButton:
            self._push_undo()
            img_pos = self.view_to_image(event.position())
            self._tool.press(img_pos)
        elif event.button() == Qt.MouseButton.RightButton:
            self._panning = True
            self._pan_start = event.position()

    def mouseMoveEvent(self, event) -> None:
        self._cursor = event.position()
        if self._panning:
            delta = event.position() - self._pan_start
            self.offset += delta
            self._pan_start = event.position()
            self.update()
        else:
            if event.buttons() & Qt.MouseButton.LeftButton:
                img_pos = self.view_to_image(event.position())
                self._tool.move(img_pos)
        self.update()

    def mouseReleaseEvent(self, event) -> None:
        self._cursor = event.position()
        if event.button() == Qt.MouseButton.LeftButton:
            img_pos = self.view_to_image(event.position())
            self._tool.release(img_pos)
            self.update()
        elif event.button() == Qt.MouseButton.RightButton:
            self._panning = False

    def wheelEvent(self, event) -> None:
        self._cursor = event.position()
        if event.angleDelta().y() > 0:
            factor = 1.2
        else:
            factor = 1 / 1.2
        self.zoom_at(event.position(), factor)
