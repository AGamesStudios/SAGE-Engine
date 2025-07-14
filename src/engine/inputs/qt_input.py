"""Qt event based input backend."""

from __future__ import annotations

from . import InputBackend
from ..extras import pyqt

pyqt.refresh()

if not pyqt.AVAILABLE:
    raise ImportError("QtInput requires PyQt6; install it with 'pip install PyQt6'")

QObject = pyqt.QtCore.QObject
QEvent = pyqt.QtCore.QEvent


class QtInput(InputBackend, QObject):
    """Keyboard and mouse input using Qt events."""

    def __init__(self, widget=None):
        QObject.__init__(self)
        self._widget = widget
        self._keys: set[int] = set()
        self._buttons: set[int] = set()
        if widget is not None:
            widget.installEventFilter(self)

    def eventFilter(self, obj, event):  # noqa: D401 - Qt signature
        t = event.type()
        if t == QEvent.Type.KeyPress:
            self._keys.add(event.key())
        elif t == QEvent.Type.KeyRelease:
            self._keys.discard(event.key())
        elif t == QEvent.Type.MouseButtonPress:
            self._buttons.add(event.button())
        elif t == QEvent.Type.MouseButtonRelease:
            self._buttons.discard(event.button())
        return False

    def poll(self) -> None:
        pass
