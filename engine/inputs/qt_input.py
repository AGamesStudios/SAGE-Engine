
"""Qt event based input backend."""

from . import InputBackend, register_input  # noqa: E402

try:
    from PyQt6.QtCore import QObject, QEvent
except Exception as exc:  # pragma: no cover - optional dependency
    raise ImportError(
        "QtInput requires PyQt6; install it with 'pip install PyQt6'"
    ) from exc


class QtInput(InputBackend, QObject):
    """Keyboard and mouse input using Qt events."""

    def __init__(self, widget=None):
        QObject.__init__(self)
        self._widget = widget
        self._keys: set[int] = set()
        self._buttons: set[int] = set()
        if widget is not None:
            widget.installEventFilter(self)

    # eventFilter records key/button state
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
        # Qt dispatches events automatically
        pass

    def is_key_down(self, key: int) -> bool:
        return key in self._keys

    def is_button_down(self, button: int) -> bool:
        return button in self._buttons

    def get_axis_value(self, axis_id: int) -> float | None:
        return None

    def shutdown(self) -> None:
        if self._widget is not None:
            self._widget.removeEventFilter(self)
        self._keys.clear()
        self._buttons.clear()


register_input("qt", QtInput)
