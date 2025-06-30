from __future__ import annotations

import os
from datetime import datetime

from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QPushButton, QListWidget, QListWidgetItem,
    QWidget, QLabel, QGroupBox, QFrame, QMenu
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont

from .icons import load_icon
from .editor import Editor, save_recent

class ProjectItem(QFrame):
    """Card widget showing project name and timestamps."""

    def __init__(self, path: str, created_label: str, last_label: str, parent=None):
        super().__init__(parent)
        self.setFrameShape(QFrame.Shape.StyledPanel)
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
        # Use no parent so the window appears in the taskbar and can be
        # restored if hidden. This avoids losing access to the launcher if the
        # hidden editor parent is not visible yet.
        super().__init__(None)
        self.editor = editor
        self.setWindowFlag(Qt.WindowType.Window, True)
        self.setWindowTitle(editor.t('project_launcher'))
        self.resize(600, 400)
        self.setMinimumSize(400, 300)
        layout = QVBoxLayout(self)
        top = QHBoxLayout()
        self.new_btn = QPushButton(load_icon('add.png'), editor.t('new_project'))
        self.open_btn = QPushButton(load_icon('open.png'), editor.t('open_project'))
        top.addWidget(self.new_btn)
        top.addWidget(self.open_btn)
        top.addStretch(1)
        layout.addLayout(top)

        group = QGroupBox(editor.t('project_list'))
        g_layout = QVBoxLayout(group)
        g_layout.setContentsMargins(0, 0, 0, 0)

        self.list = QListWidget()
        self.list.setSelectionMode(QListWidget.SelectionMode.SingleSelection)
        self.list.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.list.customContextMenuRequested.connect(self._context_menu)
        g_layout.addWidget(self.list)
        layout.addWidget(group, 1)

        self.new_btn.clicked.connect(self.create_project)
        self.open_btn.clicked.connect(self.browse_project)
        self.list.itemDoubleClicked.connect(self.open_selected)
        self.list.itemActivated.connect(self.open_selected)

        self.populate()

    def _context_menu(self, pos) -> None:
        item = self.list.itemAt(pos)
        menu = QMenu(self)
        if item and item.data(Qt.ItemDataRole.UserRole):
            del_act = menu.addAction(
                load_icon('delete.png'), self.editor.t('delete_project')
            )
            action = menu.exec(self.list.viewport().mapToGlobal(pos))
            if action == del_act:
                path = item.data(Qt.ItemDataRole.UserRole)
                if (
                    QMessageBox.question(
                        self,
                        self.editor.t('delete_project'),
                        self.editor.t('confirm_delete_project'),
                    )
                    == QMessageBox.StandardButton.Yes
                ):
                    try:
                        import shutil
                        shutil.rmtree(os.path.dirname(path))
                    except Exception as exc:
                        QMessageBox.warning(self, self.editor.t('error'), str(exc))
                    row = self.list.row(item)
                    self.list.takeItem(row)
                    if path in self.editor.recent_projects:
                        self.editor.recent_projects.remove(path)
                        save_recent(self.editor.recent_projects)
        else:
            new_act = menu.addAction(load_icon('add.png'), self.editor.t('new_project'))
            action = menu.exec(self.list.viewport().mapToGlobal(pos))
            if action == new_act:
                self.create_project()

    def add_project_card(self, path: str) -> bool:
        """Add a project entry for *path* if it exists."""
        if not os.path.exists(path):
            return False
        item = QListWidgetItem()
        widget = ProjectItem(path, self.editor.t('created'), self.editor.t('last_run'))
        item.setSizeHint(widget.sizeHint())
        item.setData(Qt.ItemDataRole.UserRole, path)
        self.list.addItem(item)
        self.list.setItemWidget(item, widget)
        return True

    def populate(self) -> None:
        self.list.clear()
        valid: list[str] = []
        for path in self.editor.recent_projects:
            if self.add_project_card(path):
                valid.append(path)
        if valid != self.editor.recent_projects:
            self.editor.recent_projects = valid
            save_recent(valid)
        if not valid:
            self.list.addItem(self.editor.t('no_recent_projects'))

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
