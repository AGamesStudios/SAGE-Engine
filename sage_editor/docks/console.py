import os
from html import escape
from PyQt6.QtWidgets import (
    QDockWidget, QTextEdit, QPushButton, QCheckBox, QWidget,
    QVBoxLayout, QHBoxLayout
)
from PyQt6.QtCore import Qt
from ..icons import load_icon

class ConsoleDock(QDockWidget):
    """Simple dock window displaying log output."""

    def __init__(self, editor):
        super().__init__(editor.t('console'), editor)
        self.editor = editor

        self.lines: list[tuple[str, str]] = []
        self.filters = {"INFO": True, "WARNING": True, "ERROR": True}

        container = QWidget()
        layout = QVBoxLayout(container)
        layout.setContentsMargins(4, 4, 4, 4)

        bar = QHBoxLayout()
        self.clear_btn = QPushButton()
        self.clear_btn.setIcon(load_icon('delete.png'))
        self.clear_btn.setToolTip(editor.t('clear_log'))
        self.clear_btn.clicked.connect(self.clear)
        self.info_chk = QCheckBox(editor.t('messages'))
        self.warn_chk = QCheckBox(editor.t('warnings'))
        self.err_chk = QCheckBox(editor.t('errors'))
        for chk in (self.info_chk, self.warn_chk, self.err_chk):
            chk.setChecked(True)
            chk.toggled.connect(self.update_display)
        bar.addWidget(self.clear_btn)
        bar.addWidget(self.info_chk)
        bar.addWidget(self.warn_chk)
        bar.addWidget(self.err_chk)
        bar.addStretch(1)
        layout.addLayout(bar)

        self.text = QTextEdit()
        self.text.setReadOnly(True)
        layout.addWidget(self.text)

        self.setWidget(container)
        editor.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self)
        self.write(f'Engine path: {os.getcwd()}')


    def clear(self) -> None:
        self.lines.clear()
        self.text.clear()

    def write(self, text: str) -> None:
        """Append *text* to the console, parsing its log level."""
        if not text.strip():
            return
        for raw in text.rstrip().splitlines():
            lvl = 'INFO'
            if 'ERROR' in raw:
                lvl = 'ERROR'
            elif 'WARNING' in raw:
                lvl = 'WARNING'
            self.lines.append((lvl, raw))
            if len(self.lines) > 1000:
                self.lines.pop(0)
            if self.filters.get(lvl, True):
                self._append_raw(lvl, raw)
        self.text.ensureCursorVisible()

    def _append_raw(self, level: str, line: str) -> None:
        """Append a line to the text widget using the current theme colors."""
        dark = getattr(self.editor, 'theme', 'dark') == 'dark'
        colors_dark = {'INFO': 'white', 'WARNING': 'orange', 'ERROR': 'red'}
        colors_light = {'INFO': 'black', 'WARNING': 'darkorange', 'ERROR': 'red'}
        palette = colors_dark if dark else colors_light
        color = palette.get(level, palette['INFO'])
        self.text.append(f'<span style="color:{color}">{escape(line)}</span>')

    def update_display(self) -> None:
        self.filters['INFO'] = self.info_chk.isChecked()
        self.filters['WARNING'] = self.warn_chk.isChecked()
        self.filters['ERROR'] = self.err_chk.isChecked()
        self.text.clear()
        for lvl, line in self.lines:
            if self.filters.get(lvl, True):
                self._append_raw(lvl, line)
        self.text.ensureCursorVisible()

