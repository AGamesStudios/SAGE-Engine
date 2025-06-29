from __future__ import annotations

from PyQt6.QtGui import QPainter, QPen, QColor
from PyQt6.QtCore import Qt, QPoint


class Tool:
    """Base class for painting tools."""

    def __init__(self, canvas: 'Canvas'):
        self.canvas = canvas

    def pen(self) -> QPen:
        return QPen(
            self.canvas.pen_color,
            self.canvas.pen_width,
            join=Qt.PenJoinStyle.RoundJoin,
            cap=Qt.PenCapStyle.RoundCap,
        )

    def press(self, pos: QPoint) -> None:
        pass

    def move(self, pos: QPoint) -> None:
        pass

    def release(self, pos: QPoint) -> None:
        pass

    def draw_gizmo(self, painter: QPainter, pos: QPoint) -> None:
        """Optionally render a representation of the tool at ``pos``."""
        del painter, pos


class BrushTool(Tool):
    def __init__(self, canvas: 'Canvas', shape: str = 'circle'):
        super().__init__(canvas)
        self.shape = shape
        self._last = QPoint()
        self._drawing = False

    def set_shape(self, shape: str) -> None:
        self.shape = shape

    def pen(self) -> QPen:
        join = Qt.PenJoinStyle.RoundJoin
        cap = Qt.PenCapStyle.RoundCap
        if self.shape == 'square':
            join = Qt.PenJoinStyle.MiterJoin
            cap = Qt.PenCapStyle.SquareCap
        return QPen(self.canvas.pen_color, self.canvas.pen_width, join=join, cap=cap)

    def press(self, pos: QPoint) -> None:
        self._last = pos
        self._drawing = True

    def move(self, pos: QPoint) -> None:
        if not self._drawing:
            return
        painter = QPainter(self.canvas.image)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, self.canvas.smooth_pen)
        painter.setRenderHint(QPainter.RenderHint.HighQualityAntialiasing, self.canvas.smooth_pen)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform, self.canvas.smooth_pen)
        painter.setPen(self.pen())
        painter.drawLine(self._last, pos)
        painter.end()
        self._last = pos

    def release(self, pos: QPoint) -> None:
        if self._drawing:
            self.move(pos)
            self._drawing = False

    def draw_gizmo(self, painter: QPainter, pos: QPoint) -> None:
        painter.save()
        pen = QPen(Qt.GlobalColor.black)
        pen.setStyle(Qt.PenStyle.DotLine)
        pen.setWidth(1)
        painter.setPen(pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        r = self.canvas.pen_width / 2
        if self.shape == 'square':
            painter.drawRect(int(pos.x() - r), int(pos.y() - r), int(self.canvas.pen_width), int(self.canvas.pen_width))
        else:
            painter.drawEllipse(pos, int(r), int(r))
        painter.restore()


class EraserTool(BrushTool):
    def pen(self) -> QPen:
        # Draw with background color to simulate erasing
        pen = super().pen()
        pen.setColor(QColor('white'))
        return pen

    def draw_gizmo(self, painter: QPainter, pos: QPoint) -> None:
        painter.save()
        pen = QPen(Qt.GlobalColor.black)
        pen.setStyle(Qt.PenStyle.DotLine)
        pen.setWidth(1)
        painter.setPen(pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        r = self.canvas.pen_width / 2
        painter.drawRect(int(pos.x() - r), int(pos.y() - r), int(self.canvas.pen_width), int(self.canvas.pen_width))
        painter.restore()


class FillTool(Tool):
    """Simple flood fill tool."""

    def press(self, pos: QPoint) -> None:
        self._flood_fill(pos.x(), pos.y())

    def _flood_fill(self, x: int, y: int) -> None:
        image = self.canvas.image
        w, h = image.width(), image.height()
        target = image.pixelColor(x, y)
        new = self.canvas.pen_color
        if target == new:
            return
        stack = [(x, y)]
        while stack:
            px, py = stack.pop()
            if px < 0 or py < 0 or px >= w or py >= h:
                continue
            if image.pixelColor(px, py) != target:
                continue
            image.setPixelColor(px, py, new)
            stack.extend([(px + 1, py), (px - 1, py), (px, py + 1), (px, py - 1)])
