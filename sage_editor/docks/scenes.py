from PyQt6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QListWidget,
    QPushButton, QListWidgetItem
)
from PyQt6.QtCore import Qt


class ScenesDock(QDockWidget):
    """Simple dock showing available scenes."""

    def __init__(self, editor):
        super().__init__(editor.t('scenes'), editor)
        self.editor = editor
        widget = QWidget()
        layout = QVBoxLayout(widget)
        self.list = QListWidget()
        self.list.itemDoubleClicked.connect(self._open)
        layout.addWidget(self.list)
        row = QHBoxLayout()
        self.new_btn = QPushButton(editor.t('new_scene'))
        self.del_btn = QPushButton(editor.t('delete'))
        self.new_btn.clicked.connect(editor.new_scene)
        self.del_btn.clicked.connect(editor.delete_selected_scene)
        row.addWidget(self.new_btn)
        row.addWidget(self.del_btn)
        layout.addLayout(row)
        self.setWidget(widget)

    def _open(self, item: QListWidgetItem):  # pragma: no cover - UI
        path = item.data(Qt.ItemDataRole.UserRole)
        if path:
            self.editor.open_scene_tab(path)
