from __future__ import annotations

from PyQt6.QtGui import QPainter, QPen, QColor
from PyQt6.QtCore import Qt, QPoint, QRectF, QRect


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
        hqa = getattr(QPainter.RenderHint, 'HighQualityAntialiasing', None)
        if hqa is not None:
            painter.setRenderHint(hqa, self.canvas.smooth_pen)
        painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform, self.canvas.smooth_pen)
        painter.setPen(self.pen())
        painter.drawLine(self._last, pos)
        painter.end()
        dirty = QRectF(QRect(self._last, pos)).normalized()
        w = self.canvas.pen_width
        dirty = dirty.adjusted(-w, -w, w, w)
        self.canvas.update(self.canvas.image_to_view_rect(dirty))
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
        self.canvas.update()

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


class SelectTool(Tool):
    """Rectangular selection and move tool."""

    def __init__(self, canvas: 'Canvas'):
        super().__init__(canvas)
        self._start = QPoint()
        self._dragging = False
        self._moving = False
        self._base_image: QImage | None = None
        self._sel_image: QImage | None = None
        self._offset = QPoint()

    def press(self, pos: QPoint) -> None:
        sel = self.canvas.selection
        if sel is not None and sel.contains(pos):
            self._moving = True
            self._start = pos
            self._offset = QPoint()
            self._base_image = self.canvas.image.copy()
            self._sel_image = self.canvas.image.copy(sel)
        else:
            self.canvas.selection = None
            self._start = pos
            self._dragging = True

    def move(self, pos: QPoint) -> None:
        if self._dragging:
            self.canvas.selection = QRect(self._start, pos).normalized()
            self.canvas.update()
        elif self._moving and self._base_image and self._sel_image:
            delta = pos - self._start
            if delta == self._offset:
                return
            self._offset = delta
            self.canvas.image = self._base_image.copy()
            painter = QPainter(self.canvas.image)
            painter.drawImage(self.canvas.selection.topLeft() + delta, self._sel_image)
            painter.end()
            self.canvas.update()

    def release(self, pos: QPoint) -> None:
        if self._dragging:
            self.canvas.selection = QRect(self._start, pos).normalized()
            self._dragging = False
            self.canvas.update()
        elif self._moving and self._base_image and self._sel_image:
            delta = pos - self._start
            self.canvas.image = self._base_image
            painter = QPainter(self.canvas.image)
            painter.drawImage(self.canvas.selection.topLeft() + delta, self._sel_image)
            painter.end()
            self.canvas.selection = QRect(
                self.canvas.selection.topLeft() + delta,
                self._sel_image.rect().size(),
            )
            self._moving = False
            self.canvas.update()

