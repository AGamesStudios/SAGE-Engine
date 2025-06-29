from PyQt6.QtCore import Qt, QObject
from ..inputs import InputBackend, register_input


class QtInput(QObject, InputBackend):
    """Keyboard and mouse input using Qt events."""

    __slots__ = ("widget", "_keys", "_buttons", "_pos", "_wheel")

    def __init__(self, widget):
        super().__init__(widget)
        self.widget = widget
        self._keys = set()
        self._buttons = set()
        self._pos = (0, 0)
        self._wheel = 0
        widget.installEventFilter(self)

    # event filter handles key and mouse events
    def eventFilter(self, obj, event):
        t = event.type()
        if t == event.Type.KeyPress:
            self._keys.add(event.key())
        elif t == event.Type.KeyRelease:
            self._keys.discard(event.key())
        elif t == event.Type.MouseButtonPress:
            self._buttons.add(event.button())
        elif t == event.Type.MouseButtonRelease:
            self._buttons.discard(event.button())
        elif t == event.Type.MouseMove:
            pos = event.position()
            self._pos = (pos.x(), pos.y())
        elif t == event.Type.Wheel:
            self._wheel += event.angleDelta().y() / 120
        return False

    def poll(self):
        # Qt processes events automatically via its loop
        pass

    def is_key_down(self, key):
        return key in self._keys

    def is_button_down(self, button):
        return button in self._buttons

    @property
    def mouse_position(self):
        """Return the last mouse position."""
        return self._pos

    def wheel_delta(self):
        """Return and reset accumulated mouse wheel delta."""
        delta = self._wheel
        self._wheel = 0
        return delta

    def shutdown(self):
        self.widget.removeEventFilter(self)


register_input("qt", QtInput)
