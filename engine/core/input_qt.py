from PyQt6.QtCore import Qt, QObject


class QtInput(QObject):
    """Keyboard and mouse input using Qt events."""

    __slots__ = ("widget", "_keys", "_buttons")

    def __init__(self, widget):
        super().__init__(widget)
        self.widget = widget
        self._keys = set()
        self._buttons = set()
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
        return False

    def poll(self):
        # Qt processes events automatically via its loop
        pass

    def is_key_down(self, key):
        return key in self._keys

    def is_button_down(self, button):
        return button in self._buttons

    def shutdown(self):
        self.widget.removeEventFilter(self)
