from __future__ import annotations

import math

try:  # pragma: no cover - fallback when Qt not available
    from PyQt6.QtWidgets import QWidget  # type: ignore[import-not-found]
    from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - test stubs
    class QWidget:  # type: ignore[misc]
        def __init__(self, *a, **k):
            pass
    class Qt:  # type: ignore[misc]
        class Orientation:
            Horizontal = 1
            Vertical = 2

try:  # pragma: no cover - optional painting
    from PyQt6.QtGui import QPainter, QColor, QPen  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - stub fallback
    QPainter = None  # type: ignore[assignment]
    QColor = None  # type: ignore[assignment]
    QPen = None  # type: ignore[assignment]

ACCENT_COLOR = "#ffb84d"


class RulerWidget(QWidget):
    """Simple ruler with tick marks for the viewport edges."""

    def __init__(self, orientation: Qt.Orientation, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.orientation = orientation
        self.offset = 0.0
        self.scale = 1.0
        self.cursor: float | None = None
        self.sign = 1
        if orientation == getattr(Qt.Orientation, "Horizontal", 1):
            if hasattr(self, "setFixedHeight"):
                self.setFixedHeight(20)
        else:
            if hasattr(self, "setFixedWidth"):
                self.setFixedWidth(20)

    def set_transform(self, offset: float, scale: float, cursor: float | None, sign: int = 1) -> None:
        """Update the world offset/scale and cursor position."""
        self.offset = offset
        self.scale = scale or 1.0
        self.cursor = cursor
        self.sign = 1 if sign >= 0 else -1
        if hasattr(self, "update"):
            self.update()

    def paintEvent(self, event):  # pragma: no cover - visual detail
        if QPainter is None or QColor is None or QPen is None:
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        painter.fillRect(self.rect(), QColor("#2a2a2a"))
        pen = QPen(QColor("#888"))
        painter.setPen(pen)
        font = painter.font()
        font.setPointSize(7)
        painter.setFont(font)

        def step_pixels(step: float) -> float:
            return step * self.scale

        step = 1.0
        while step_pixels(step) < 10:
            step *= 2
        while step_pixels(step) > 80 and step > 1:
            step /= 2

        if self.orientation == getattr(Qt.Orientation, "Horizontal", 1):
            h = self.height()
            start = math.floor(self.offset / step) * step
            x = (start - self.offset) * self.scale
            world = start
            while x < self.width():
                pos = int(round(x))
                size = 8 if world % (step * 5) == 0 else 4
                painter.drawLine(pos, h, pos, h - size)
                if size == 8:
                    painter.drawText(pos + 2, h - size - 2, f"{world:.0f}")
                x += step_pixels(step)
                world += step
            if self.cursor is not None:
                cx = (self.cursor - self.offset) * self.scale
                if 0 <= cx <= self.width():
                    pos = int(round(cx))
                    painter.setPen(QPen(QColor(ACCENT_COLOR)))
                    painter.drawLine(pos, 0, pos, h)
        else:
            w = self.width()
            start = math.floor(self.offset / step) * step
            y = (start - self.offset) * self.scale * self.sign
            world = start
            while y < self.height():
                size = 8 if world % (step * 5) == 0 else 4
                painter.drawLine(w, int(round(y)), w - size, int(round(y)))
                if size == 8:
                    painter.drawText(2, int(round(y)) + 6, f"{world:.0f}")
                y += step_pixels(step)
                world += step * self.sign
            if self.cursor is not None:
                cy = (self.cursor - self.offset) * self.scale * self.sign
                if 0 <= cy <= self.height():
                    painter.setPen(QPen(QColor(ACCENT_COLOR)))
                    painter.drawLine(0, int(round(cy)), w, int(round(cy)))

