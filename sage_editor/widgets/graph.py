from __future__ import annotations

from collections import deque
from typing import Deque

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QPainter, QColor, QPaintEvent
from PyQt6.QtWidgets import QWidget


class GraphWidget(QWidget):
    """Simple line graph widget with a fixed 0-100 y-range."""

    HISTORY = 60

    def __init__(self, title: str, color: QColor, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.title = title
        self.color = color
        self.data: Deque[float] = deque(maxlen=self.HISTORY)

    def append(self, value: float) -> None:
        self.data.append(value)
        self.update()

    def paintEvent(self, event: QPaintEvent) -> None:  # pragma: no cover - UI drawing
        painter = QPainter(self)
        painter.fillRect(event.rect(), self.palette().base())
        painter.setPen(Qt.GlobalColor.white)
        painter.drawText(4, 14, self.title)
        if len(self.data) < 2:
            return
        painter.setPen(self.color)
        w = self.width() - 4
        h = self.height() - 20
        if h <= 0:
            return
        max_idx = len(self.data) - 1
        prev_x = 4
        prev_y = 20 + h - (self.data[0] / 100) * h
        for i, val in enumerate(self.data):
            x = 4 + (i / max_idx) * w
            y = 20 + h - (val / 100) * h
            painter.drawLine(int(prev_x), int(prev_y), int(x), int(y))
            prev_x, prev_y = x, y
