import os
from PyQt6.QtWidgets import QDockWidget, QTextEdit
from PyQt6.QtCore import Qt

class ConsoleDock(QDockWidget):
    """Simple dock window displaying log output."""

    def __init__(self, editor):
        super().__init__(editor.t('console'), editor)
        self.editor = editor
        self.text = QTextEdit()
        self.text.setReadOnly(True)
        self.setWidget(self.text)
        # default location at the bottom of the main window
        editor.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self)
        self.text.append(f'Engine path: {os.getcwd()}')

