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


class EraserTool(BrushTool):
    def pen(self) -> QPen:
        # Draw with background color to simulate erasing
        pen = super().pen()
        pen.setColor(QColor('white'))
        return pen
