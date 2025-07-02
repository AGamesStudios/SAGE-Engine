from __future__ import annotations

from PyQt6.QtWidgets import QPlainTextEdit
from PyQt6.QtGui import QTextCursor


class ConsoleWidget(QPlainTextEdit):
    """Simple widget that displays log output."""

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self.setReadOnly(True)
        self.setStyleSheet(
            "QPlainTextEdit {background:#1e1e1e;color:#dcdcdc;font-family:monospace;}"
        )

    def write(self, text: str) -> None:
        """Append *text* to the console."""
        self.moveCursor(QTextCursor.MoveOperation.End)
        self.insertPlainText(text)
        self.moveCursor(QTextCursor.MoveOperation.End)

