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
    def __init__(self, canvas: 'Canvas'):
        super().__init__(canvas)
        self._last = QPoint()
        self._drawing = False

    def press(self, pos: QPoint) -> None:
        self._last = pos
        self._drawing = True

    def move(self, pos: QPoint) -> None:
        if not self._drawing:
            return
        painter = QPainter(self.canvas.image)
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
        painter.drawEllipse(pos, r, r)
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
        painter.drawRect(pos.x() - r, pos.y() - r, self.canvas.pen_width, self.canvas.pen_width)
        painter.restore()
