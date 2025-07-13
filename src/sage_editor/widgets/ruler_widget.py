from __future__ import annotations

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


class RulerWidget(QWidget):
    """Simple ruler with tick marks for the viewport edges."""

    def __init__(self, orientation: Qt.Orientation, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.orientation = orientation
        if orientation == getattr(Qt.Orientation, "Horizontal", 1):
            if hasattr(self, "setFixedHeight"):
                self.setFixedHeight(20)
        else:
            if hasattr(self, "setFixedWidth"):
                self.setFixedWidth(20)

    def paintEvent(self, event):  # pragma: no cover - visual detail
        if QPainter is None or QColor is None or QPen is None:
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        pen = QPen(QColor("#888"))
        painter.setPen(pen)
        if self.orientation == getattr(Qt.Orientation, "Horizontal", 1):
            h = self.height()
            for x in range(0, self.width(), 10):
                size = 8 if x % 50 == 0 else 4
                painter.drawLine(x, h, x, h - size)
        else:
            w = self.width()
            for y in range(0, self.height(), 10):
                size = 8 if y % 50 == 0 else 4
                painter.drawLine(w, y, w - size, y)

