from __future__ import annotations

import os
from datetime import datetime

from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QPushButton, QListWidget, QListWidgetItem,
    QWidget, QLabel
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont

from .icons import load_icon
from .editor import Editor, save_recent

class ProjectItem(QWidget):
    """Simple card showing project info."""

    def __init__(self, path: str, created_label: str, last_label: str, parent=None):
        super().__init__(parent)
        self.path = path
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 4, 8, 4)
        name = os.path.splitext(os.path.basename(path))[0]
        title = QLabel(name)
        font = QFont()
        font.setPointSize(12)
        font.setBold(True)
        title.setFont(font)
        try:
            ctime = datetime.fromtimestamp(os.path.getctime(path)).strftime('%Y-%m-%d')
            ltime = datetime.fromtimestamp(os.path.getatime(path)).strftime('%Y-%m-%d')
        except Exception:
            ctime = ltime = ''
        info = QLabel(f"{created_label} {ctime}\n{last_label} {ltime}")
        layout.addWidget(title)
        layout.addWidget(info)

class ProjectHub(QDialog):
    """Experimental project launcher with big project cards."""

    def __init__(self, editor: Editor):
        super().__init__(editor)
        self.editor = editor
        self.setWindowTitle(editor.t('project_launcher'))
        self.resize(600, 400)
        layout = QVBoxLayout(self)
        top = QHBoxLayout()
        self.new_btn = QPushButton(load_icon('add.png'), editor.t('new_project'))
        self.open_btn = QPushButton(load_icon('open.png'), editor.t('open_project'))
        top.addWidget(self.new_btn)
        top.addWidget(self.open_btn)
        top.addStretch(1)
        layout.addLayout(top)

        self.list = QListWidget()
        self.list.setSelectionMode(QListWidget.SelectionMode.SingleSelection)
        layout.addWidget(self.list, 1)

        self.new_btn.clicked.connect(self.create_project)
        self.open_btn.clicked.connect(self.browse_project)
        self.list.itemDoubleClicked.connect(self.open_selected)

        self.populate()

    def populate(self) -> None:
        self.list.clear()
        valid: list[str] = []
        for path in self.editor.recent_projects:
            if not os.path.exists(path):
                continue
            item = QListWidgetItem()
            widget = ProjectItem(path, self.editor.t('created'), self.editor.t('last_run'))
            item.setSizeHint(widget.sizeHint())
            item.setData(Qt.ItemDataRole.UserRole, path)
            self.list.addItem(item)
            self.list.setItemWidget(item, widget)
            valid.append(path)
        if valid != self.editor.recent_projects:
            self.editor.recent_projects = valid
            save_recent(valid)

    def open_selected(self):
        item = self.list.currentItem()
        if not item:
            return
        path = item.data(Qt.ItemDataRole.UserRole)
        self.editor.open_project(path)
        if self.editor.project_path:
            self.accept()

    def browse_project(self):
        self.editor.open_project()
        if self.editor.project_path:
            self.accept()

    def create_project(self):
        self.editor.new_project()
        if self.editor.project_path:
            self.accept()
