"""Optional PyQt GUI wrapper."""

from __future__ import annotations

try:
    from PyQt6 import QtWidgets as QtW  # type: ignore
except Exception:
    try:
        from PyQt5 import QtWidgets as QtW  # type: ignore
    except Exception:  # PyQt not available
        QtW = None  # type: ignore


class GuiApp:
    """Simple application wrapper."""

    def __init__(self, game=None) -> None:
        self.game = game
        self._app = None
        self.window = None

    def run(self, headless: bool = False) -> None:
        if QtW is None or headless:
            return
        if self._app is None:
            self._app = QtW.QApplication([])
        if self.window is None:
            self.window = QtW.QWidget()
            self.window.setWindowTitle("SAGE Engine")
        self.window.show()
        self._app.exec()

    def quit(self) -> None:
        if self._app is not None:
            self._app.quit()
