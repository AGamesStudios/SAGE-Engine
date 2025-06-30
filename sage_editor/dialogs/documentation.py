from __future__ import annotations

import os

from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QListWidget, QTextEdit,
    QDialogButtonBox, QStyleFactory
)
from PyQt6.QtCore import Qt

from ..editor import BASE_DIR


class DocumentationDialog(QDialog):
    """Simple viewer for bundled documentation."""

    def __init__(self, editor):
        super().__init__(editor)
        self.editor = editor
        self.setWindowTitle(editor.t('documentation'))
        self.resize(700, 500)
        self.setStyle(QStyleFactory.create('Fusion'))
        editor.apply_theme()

        self.docs: dict[str, str] = {}
        lang_dir = os.path.join(BASE_DIR, 'docs', editor.lang)
        if not os.path.isdir(lang_dir):
            lang_dir = os.path.join(BASE_DIR, 'docs', 'en')
        for fname in os.listdir(lang_dir):
            if fname.endswith('.md'):
                name = os.path.splitext(fname)[0].replace('_', ' ').title()
                self.docs[name] = os.path.join(lang_dir, fname)

        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        top = QHBoxLayout()
        self.list = QListWidget()
        self.list.setFixedWidth(150)
        for name in sorted(self.docs):
            self.list.addItem(name)
        self.text = QTextEdit()
        self.text.setReadOnly(True)
        top.addWidget(self.list)
        top.addWidget(self.text, 1)
        layout.addLayout(top)

        btns = QDialogButtonBox(QDialogButtonBox.StandardButton.Close)
        btns.rejected.connect(self.reject)
        layout.addWidget(btns)

        self.list.currentTextChanged.connect(self.load_doc)
        if self.list.count():
            self.list.setCurrentRow(0)

        editor.apply_no_wheel(self)

    def load_doc(self, name: str) -> None:
        path = self.docs.get(name)
        if not path:
            self.text.clear()
            return
        try:
            with open(path, 'r', encoding='utf-8') as f:
                self.text.setMarkdown(f.read())
        except Exception as exc:
            self.text.setPlainText(str(exc))
