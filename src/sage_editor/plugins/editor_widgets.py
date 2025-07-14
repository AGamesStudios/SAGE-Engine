from __future__ import annotations

import logging
import math
import json
import zlib
from typing import Any, TYPE_CHECKING

from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
from PyQt6.QtWidgets import (
    QLabel,
    QDoubleSpinBox,
    QHBoxLayout,
    QLineEdit,
    QPushButton,
    QVBoxLayout,
    QWidget,
)

if TYPE_CHECKING:  # pragma: no cover - for type hints
    from engine.core.scenes.scene import Scene
    from sage_editor.plugins.editor_window import EditorWindow

try:  # optional for tests
    from PyQt6.QtWidgets import QAbstractSpinBox  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback when class missing
    class QAbstractSpinBox:  # type: ignore[too-few-public-methods]
        class ButtonSymbols:
            NoButtons = 0


class ConsoleHandler(logging.Handler):
    """Forward log records to a text widget."""

    def __init__(self, widget: object) -> None:
        super().__init__()
        self.widget: Any = widget

    def emit(self, record: logging.LogRecord) -> None:  # pragma: no cover - UI handler
        msg = self.format(record)
        color = "#dddddd"
        if record.levelno >= logging.ERROR:
            color = "#ff5555"
        elif record.levelno >= logging.WARNING:
            color = "#ffaa00"
        if hasattr(self.widget, "appendHtml"):
            self.widget.appendHtml(f"<span style='color:{color}'>{msg}</span>")
        elif hasattr(self.widget, "append"):
            self.widget.append(msg)
        else:
            text = getattr(self.widget, "toPlainText", lambda: "")()
            if hasattr(self.widget, "setPlainText"):
                self.widget.setPlainText(text + msg + "\n")


class UndoStack:
    """Simple stack for undo/redo snapshots."""

    def __init__(self) -> None:
        self._undo: list[bytes] = []
        self._redo: list[bytes] = []

    def snapshot(self, scene: "Scene") -> None:
        data = json.dumps(scene.to_dict()).encode("utf-8")
        self._undo.append(zlib.compress(data))
        self._redo.clear()

    def undo(self, window: "EditorWindow") -> None:
        if not self._undo:
            return
        state = json.loads(zlib.decompress(self._undo.pop()).decode("utf-8"))
        self._redo.append(zlib.compress(json.dumps(window.scene.to_dict()).encode("utf-8")))
        window.scene = Scene.from_dict(state)
        window.update_object_list(preserve=False)
        window.draw_scene()

    def redo(self, window: "EditorWindow") -> None:
        if not self._redo:
            return
        state = json.loads(zlib.decompress(self._redo.pop()).decode("utf-8"))
        self._undo.append(zlib.compress(json.dumps(window.scene.to_dict()).encode("utf-8")))
        window.scene = Scene.from_dict(state)
        window.update_object_list(preserve=False)
        window.draw_scene()


class TransformBar(QWidget):
    """Simple vertical bar with toggle buttons for transform modes."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(0, 0, 0, 0)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(6)
        self.move_btn = QPushButton("Move", self)
        self.rotate_btn = QPushButton("Rotate", self)
        self.scale_btn = QPushButton("Scale", self)
        self.rect_btn = QPushButton("Rect", self)
        self.local_btn = QPushButton("Local", self)
        for btn in [
            self.move_btn,
            self.rotate_btn,
            self.scale_btn,
            self.rect_btn,
            self.local_btn,
        ]:
            if hasattr(btn, "setCheckable"):
                btn.setCheckable(True)
            layout.addWidget(btn)
        if hasattr(layout, "addStretch"):
            layout.addStretch()


class ModelBar(QWidget):
    """Extra buttons for modeling operations."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(0, 0, 0, 0)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(6)
        self.vert_btn = QPushButton("Verts", self)
        self.edge_btn = QPushButton("Edges", self)
        self.face_btn = QPushButton("Faces", self)
        self.extrude_btn = QPushButton("Extrude", self)
        self.loop_btn = QPushButton("Loop Cut", self)
        self.fill_btn = QPushButton("Fill", self)
        self.union_btn = QPushButton("Union", self)
        for btn in [self.vert_btn, self.edge_btn, self.face_btn]:
            if hasattr(btn, "setCheckable"):
                btn.setCheckable(True)
            layout.addWidget(btn)
        layout.addWidget(self.extrude_btn)
        layout.addWidget(self.loop_btn)
        layout.addWidget(self.fill_btn)
        layout.addWidget(self.union_btn)
        if hasattr(layout, "addStretch"):
            layout.addStretch()


class ProgressWheel(QWidget):
    """Custom circular control that shows and edits an angle."""

    try:  # pragma: no cover - signal for real Qt only
        from PyQt6.QtCore import pyqtSignal  # type: ignore[import-not-found]
    except Exception:  # pragma: no cover - stub signal
        def pyqtSignal(*_a, **_k):  # type: ignore[return-type]
            class _DummySignal:
                def connect(self, *a, **k):
                    pass

            return _DummySignal()

    valueChanged = pyqtSignal(int)  # type: ignore[assignment]

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        layout = QHBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(0, 0, 0, 0)
        self._label = QLabel("0", self)
        if hasattr(self._label, "setAlignment"):
            self._label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self._label)
        self._minimum = 0
        self._maximum = 360
        self._value = 0
        self._drag_start: tuple[float, float] | None = None
        self._start_value = 0
        self._update_label(self._value)

    # basic API -----------------------------------------------------
    def value(self) -> int:
        return self._value

    def setRange(self, minimum: int, maximum: int) -> None:
        self._minimum = minimum
        self._maximum = maximum
        self.setValue(self._value)

    def setValue(self, value: int) -> None:
        value = max(self._minimum, min(self._maximum, int(value)))
        if value != self._value:
            self._value = value
            self._update_label(value)
            self.valueChanged.emit(value)
            if hasattr(self, "update"):
                self.update()

    # painting ------------------------------------------------------
    def paintEvent(self, event):  # pragma: no cover - visual
        try:
            from PyQt6.QtGui import QPainter, QColor, QPen  # type: ignore[import-not-found]
        except Exception:
            return
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        size = min(self.width(), self.height()) - 6
        rect = type(self.rect())(
            (self.width() - size) / 2,
            (self.height() - size) / 2,
            size,
            size,
        )
        pen = QPen(QColor("#555555"), 4)
        painter.setPen(pen)
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawEllipse(rect)
        pen.setColor(QColor(255, 184, 77))
        painter.setPen(pen)
        painter.drawArc(rect, 90 * 16, int(-self._value * 16))

    # interaction ---------------------------------------------------
    def wheelEvent(self, event):  # pragma: no cover - ui tweak
        event.ignore()

    def mousePressEvent(self, event):  # pragma: no cover - ui tweak
        if event.button() == Qt.MouseButton.LeftButton:
            self._drag_start = (event.position().x(), event.position().y())
            self._start_value = self._value
            event.accept()
        else:
            super().mousePressEvent(event)

    def mouseMoveEvent(self, event):  # pragma: no cover - ui tweak
        if self._drag_start is not None:
            cx, cy = self.width() / 2, self.height() / 2
            x0, y0 = self._drag_start
            ang0 = math.atan2(y0 - cy, x0 - cx)
            ang1 = math.atan2(event.position().y() - cy, event.position().x() - cx)
            delta = math.degrees(ang1 - ang0)
            self.setValue((self._start_value + int(delta)) % 360)
            event.accept()
        else:
            super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):  # pragma: no cover - ui tweak
        if self._drag_start is not None:
            self._drag_start = None
            event.accept()
        else:
            super().mouseReleaseEvent(event)

    def resizeEvent(self, event):  # pragma: no cover - gui adjustment
        super().resizeEvent(event)
        if hasattr(self._label, "setGeometry"):
            self._label.setGeometry(0, 0, self.width(), self.height())

    def _update_label(self, value: int) -> None:
        if hasattr(self._label, "setText"):
            self._label.setText(str(value))


class NoWheelLineEdit(QLineEdit):
    """Line edit that ignores mouse wheel events."""

    def wheelEvent(self, event):  # pragma: no cover - ui tweak
        event.ignore()


class NoWheelSpinBox(QDoubleSpinBox):
    """Spin box that ignores the mouse wheel and supports drag editing."""

    DRAG_SPEED = 0.05

    def __init__(self, *a, **k):
        super().__init__(*a, **k)
        if hasattr(self, "setButtonSymbols"):
            self.setButtonSymbols(QAbstractSpinBox.ButtonSymbols.NoButtons)
        self._drag_origin: float | None = None
        self._drag_value: float = 0.0

    def wheelEvent(self, event):  # pragma: no cover - ui tweak
        event.ignore()

    # Drag with the left mouse button to change the value quickly
    def mousePressEvent(self, event):  # pragma: no cover - ui tweak
        if event.button() == Qt.MouseButton.LeftButton:
            self._drag_origin = getattr(event, "globalPosition", event.pos)().y()
            self._drag_value = self.value()
            if hasattr(self, "setCursor"):
                self.setCursor(Qt.CursorShape.SizeVerCursor)
            if hasattr(self.lineEdit(), "setFocus"):
                self.lineEdit().setFocus()
            event.accept()
            return
        if hasattr(QDoubleSpinBox, "mousePressEvent"):
            QDoubleSpinBox.mousePressEvent(self, event)

    def mouseMoveEvent(self, event):  # pragma: no cover - ui tweak
        if self._drag_origin is not None and event.buttons() & Qt.MouseButton.LeftButton:
            y = getattr(event, "globalPosition", event.pos)().y()
            delta = self._drag_origin - y
            step = self.singleStep() or 1.0
            new_val = self._drag_value + delta * step * self.DRAG_SPEED
            self.setValue(new_val)
            event.accept()
            return
        if hasattr(QDoubleSpinBox, "mouseMoveEvent"):
            QDoubleSpinBox.mouseMoveEvent(self, event)

    def mouseReleaseEvent(self, event):  # pragma: no cover - ui tweak
        if event.button() == Qt.MouseButton.LeftButton and self._drag_origin is not None:
            self._drag_origin = None
            if hasattr(self, "unsetCursor"):
                self.unsetCursor()
        if hasattr(QDoubleSpinBox, "mouseReleaseEvent"):
            QDoubleSpinBox.mouseReleaseEvent(self, event)


class SnapSettingsWidget(QWidget):
    """Dockable widget for configuring grid snapping."""

    def __init__(self, window: "EditorWindow", parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._window = window
        try:  # pragma: no cover - present in real Qt only
            from PyQt6.QtWidgets import QFormLayout  # type: ignore[import-not-found]
        except Exception:
            QFormLayout = QVBoxLayout  # type: ignore[assignment]

        try:  # pragma: no cover - present in real Qt only
            from PyQt6.QtWidgets import QFrame  # type: ignore[import-not-found]
        except Exception:
            QFrame = QWidget  # type: ignore[assignment]

        layout = QFormLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(4, 4, 4, 4)

        self.move_spin = NoWheelSpinBox(self)
        self.move_spin.setDecimals(3)
        self.move_spin.setRange(0.01, 100.0)
        self.move_spin.setValue(window.move_step)
        layout.addRow("Move Step", self.move_spin)
        line = QFrame(self)
        if hasattr(line, "setFrameShape"):
            line.setFrameShape(QFrame.Shape.HLine)
        layout.addRow(line)

        self.rot_spin = NoWheelSpinBox(self)
        self.rot_spin.setDecimals(1)
        self.rot_spin.setRange(0.1, 360.0)
        self.rot_spin.setValue(window.rotate_step)
        layout.addRow("Rotate Step", self.rot_spin)
        line = QFrame(self)
        if hasattr(line, "setFrameShape"):
            line.setFrameShape(QFrame.Shape.HLine)
        layout.addRow(line)

        self.scale_spin = NoWheelSpinBox(self)
        self.scale_spin.setDecimals(3)
        self.scale_spin.setRange(0.01, 10.0)
        self.scale_spin.setValue(window.scale_step)
        layout.addRow("Scale Step", self.scale_spin)

        for spin in [self.move_spin, self.rot_spin, self.scale_spin]:
            if hasattr(spin, "valueChanged"):
                spin.valueChanged.connect(self._apply)

    def _apply(self) -> None:
        self._window.move_step = float(self.move_spin.value())
        self._window.rotate_step = float(self.rot_spin.value())
        self._window.scale_step = float(self.scale_spin.value())


class SnapPopup(QWidget):
    """Floating menu that shows snap step controls next to the toolbar."""

    def __init__(self, window: "EditorWindow", parent: QWidget | None = None) -> None:
        flag = getattr(getattr(Qt, "WindowType", Qt), "Popup", getattr(Qt, "Popup", 0))
        super().__init__(parent, flag)
        if hasattr(self, "setStyleSheet"):
            self.setStyleSheet(
                "background:#333;border:2px solid white;border-radius:4px;",
            )
        layout = QVBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(8, 8, 8, 8)
        self.settings = SnapSettingsWidget(window, self)
        layout.addWidget(self.settings)

    def show_near(self, widget: QWidget) -> None:
        pos = widget.mapToGlobal(widget.rect().bottomLeft())
        if hasattr(self, "move"):
            self.move(pos)
        if hasattr(self, "show"):
            self.show()

