from __future__ import annotations

import os
import shutil

from PyQt6.QtWidgets import (
    QDialog, QVBoxLayout, QHBoxLayout, QPushButton,
    QTableWidget, QTableWidgetItem, QFileDialog, QMessageBox
)
from PyQt6.QtCore import Qt

from sage_sdk import plugins


class PluginManager(QDialog):
    """Dialog to install and toggle plugins."""

    def __init__(self, editor):
        super().__init__(editor)
        self.editor = editor
        self.setWindowTitle(editor.t('manage_plugins'))
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(6)

        self.table = QTableWidget(0, 2)
        self.table.setHorizontalHeaderLabels([editor.t('plugin'), editor.t('enabled')])
        self.table.verticalHeader().hide()
        layout.addWidget(self.table)

        btn_row = QHBoxLayout()
        btn_row.setSpacing(6)
        self.add_btn = QPushButton(editor.t('add_plugin'))
        self.del_btn = QPushButton(editor.t('delete'))
        self.close_btn = QPushButton(editor.t('close'))
        btn_row.addWidget(self.add_btn)
        btn_row.addWidget(self.del_btn)
        btn_row.addStretch(1)
        btn_row.addWidget(self.close_btn)
        layout.addLayout(btn_row)

        self.add_btn.clicked.connect(self.add_plugin)
        self.del_btn.clicked.connect(self.remove_plugin)
        self.close_btn.clicked.connect(self.accept)

        self.populate()

    def populate(self):
        self.table.setRowCount(0)
        for name, path, enabled in plugins.list_plugins():
            row = self.table.rowCount()
            self.table.insertRow(row)
            item = QTableWidgetItem(name)
            item.setToolTip(path)
            flag = Qt.ItemFlag.ItemIsUserCheckable | Qt.ItemFlag.ItemIsEnabled
            chk = QTableWidgetItem()
            chk.setFlags(flag)
            chk.setCheckState(Qt.CheckState.Checked if enabled else Qt.CheckState.Unchecked)
            self.table.setItem(row, 0, item)
            self.table.setItem(row, 1, chk)

    def add_plugin(self):
        path, _ = QFileDialog.getOpenFileName(
            self,
            self.editor.t('select_file'),
            '',
            self.editor.t('py_files'),
        )
        if not path:
            return
        os.makedirs(plugins.PLUGIN_DIR, exist_ok=True)
        dest = os.path.join(plugins.PLUGIN_DIR, os.path.basename(path))
        try:
            shutil.copy(path, dest)
        except Exception as exc:
            QMessageBox.warning(self, self.editor.t('error'), str(exc))
            return
        self.populate()

    def remove_plugin(self):
        row = self.table.currentRow()
        if row < 0:
            return
        path = self.table.item(row, 0).toolTip()
        name = self.table.item(row, 0).text()
        try:
            os.remove(path)
        except Exception as exc:
            QMessageBox.warning(self, self.editor.t('error'), str(exc))
            return
        cfg = plugins.read_config()
        cfg.pop(name, None)
        plugins.write_config(cfg)
        self.populate()

    def accept(self):
        cfg = plugins.read_config()
        for row in range(self.table.rowCount()):
            name = self.table.item(row, 0).text()
            enabled = self.table.item(row, 1).checkState() == Qt.CheckState.Checked
            cfg[name] = enabled
        plugins.write_config(cfg)
        super().accept()
        # reload plugins after modifications
        plugins.load_plugins('editor', self.editor)
