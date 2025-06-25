"""Standalone application entry point and project manager."""

from __future__ import annotations

import sys
import os
import traceback

from PyQt6.QtWidgets import (
    QApplication, QDialog, QVBoxLayout, QLabel, QTableWidget, QTableWidgetItem,
    QPushButton, QMenu, QHBoxLayout, QAbstractItemView, QHeaderView, QMessageBox,
    QGroupBox, QDialogButtonBox
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QPalette, QColor, QFont

STYLE_SHEET = """
QToolBar { icon-size: 24px; spacing: 6px; }
QDockWidget::title { padding: 4px; background: #333; color: #ddd; }
QGroupBox { margin-top: 8px; }
QGroupBox::title { subcontrol-origin: margin; left: 4px; padding: 0 2px; }
QPushButton { padding: 4px 8px; }
QLineEdit { padding: 2px 4px; }
QListWidget { background: #222; }
"""

from .editor import Editor, save_recent, load_recent, _log, logger


class ProjectManager(QDialog):
    """Startup dialog listing recent projects and actions."""

    def __init__(self, editor: Editor):
        super().__init__(editor)
        self.editor = editor
        self.setWindowTitle(editor.t('project_manager'))
        self.resize(720, 420)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(12, 12, 12, 12)
        layout.setSpacing(10)

        info = QLabel(editor.t('select_project_info'))
        info.setWordWrap(True)
        layout.addWidget(info)

        group = QGroupBox(editor.t('recent_projects'))
        table_layout = QVBoxLayout(group)
        table_layout.setContentsMargins(0, 0, 0, 0)

        self.table = QTableWidget(0, 4)
        self.table.setHorizontalHeaderLabels([
            editor.t('current'), editor.t('name'),
            editor.t('created'), editor.t('path')
        ])
        self.table.verticalHeader().hide()
        self.table.setSelectionBehavior(QAbstractItemView.SelectionBehavior.SelectRows)
        header = self.table.horizontalHeader()
        header_font = QFont()
        header_font.setBold(True)
        header.setFont(header_font)
        header.setSectionResizeMode(0, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(2, QHeaderView.ResizeMode.ResizeToContents)
        header.setSectionResizeMode(3, QHeaderView.ResizeMode.Stretch)
        self.table.verticalHeader().setDefaultSectionSize(22)
        self.table.setAlternatingRowColors(True)
        self.table.setEditTriggers(QAbstractItemView.EditTrigger.NoEditTriggers)
        self.table.setShowGrid(False)
        self.table.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.table.customContextMenuRequested.connect(self.show_context_menu)
        table_layout.addWidget(self.table)
        layout.addWidget(group)

        btn_box = QDialogButtonBox()
        self.new_btn = btn_box.addButton(
            editor.t('new_project'), QDialogButtonBox.ButtonRole.ActionRole
        )
        self.open_btn = btn_box.addButton(
            editor.t('open_project'), QDialogButtonBox.ButtonRole.ActionRole
        )
        self.clear_btn = btn_box.addButton(
            editor.t('clear_recent'), QDialogButtonBox.ButtonRole.ActionRole
        )
        self.cancel_btn = btn_box.addButton(
            editor.t('cancel'), QDialogButtonBox.ButtonRole.RejectRole
        )
        layout.addWidget(btn_box)

        self.new_btn.clicked.connect(self.create_project)
        self.open_btn.clicked.connect(self.browse_project)
        self.clear_btn.clicked.connect(self.clear_list)
        self.cancel_btn.clicked.connect(self.reject)
        self.table.itemDoubleClicked.connect(self.open_selected)

        self.populate()

    def show_context_menu(self, pos):
        row = self.table.indexAt(pos).row()
        if row < 0:
            return
        menu = QMenu(self)
        open_act = menu.addAction(self.editor.t('open'))
        del_act = menu.addAction(self.editor.t('delete'))
        action = menu.exec(self.table.mapToGlobal(pos))
        if action == open_act:
            self.table.selectRow(row)
            self.open_selected()
        elif action == del_act:
            path = self.table.item(row, 3).text()
            dir_path = os.path.dirname(path)
            reply = QMessageBox.question(
                self,
                self.editor.t('delete_project'),
                self.editor.t('confirm_delete_project'),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
            )
            if reply == QMessageBox.StandardButton.Yes:
                try:
                    import shutil
                    shutil.rmtree(dir_path, ignore_errors=True)
                except Exception as exc:
                    QMessageBox.warning(self, self.editor.t('error'), str(exc))
                if path in self.editor.recent_projects:
                    self.editor.recent_projects.remove(path)
                    save_recent(self.editor.recent_projects)
                self.populate()

    def populate(self):
        from datetime import datetime
        self.table.setRowCount(0)
        valid = []
        for path in self.editor.recent_projects:
            if not os.path.exists(path):
                continue
            row = self.table.rowCount()
            self.table.insertRow(row)
            current = '✓' if path == self.editor.project_path else ''
            name = os.path.splitext(os.path.basename(path))[0]
            try:
                ts = os.path.getctime(path)
                created = datetime.fromtimestamp(ts).strftime('%Y-%m-%d')
            except Exception:
                created = ''
            self.table.setItem(row, 0, QTableWidgetItem(current))
            self.table.setItem(row, 1, QTableWidgetItem(name))
            self.table.setItem(row, 2, QTableWidgetItem(created))
            self.table.setItem(row, 3, QTableWidgetItem(path))
            valid.append(path)
        if valid != self.editor.recent_projects:
            self.editor.recent_projects = valid
            save_recent(valid)

    def open_selected(self):
        row = self.table.currentRow()
        if row < 0:
            return
        path = self.table.item(row, 3).text()
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

    def clear_list(self):
        self.editor.recent_projects = []
        save_recent([])
        self.populate()


def main(argv=None):
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    app.setStyle('Fusion')
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Base, QColor(35, 35, 35))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(255, 255, 255))
    palette.setColor(QPalette.ColorRole.ToolTipText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Text, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(0, 0, 0))
    app.setPalette(palette)
    app.setStyleSheet(STYLE_SHEET)
    font = QFont()
    font.setPointSize(font.pointSize() + 3)
    app.setFont(font)
    editor = Editor(autoshow=False)
    pm = ProjectManager(editor)
    if pm.exec() != QDialog.DialogCode.Accepted:
        app.quit()
        sys.exit(0)
        return 0
    editor.showMaximized()

    orig_out = sys.stdout
    orig_err = sys.stderr

    class _Stream:
        def __init__(self, edit, orig):
            self.edit = edit
            self.orig = orig

        def write(self, text):
            if text.strip():
                self.edit.append(text.rstrip())
            self.orig.write(text)

        def flush(self):
            self.orig.flush()

    sys.stdout = _Stream(editor.console, orig_out)
    sys.stderr = _Stream(editor.console, orig_err)

    def handle_exception(exc_type, exc, tb):
        text = ''.join(traceback.format_exception(exc_type, exc, tb))
        editor.console.append(text)
        orig_err.write(text)
        orig_err.flush()
        logger.error('Unhandled exception', exc_info=(exc_type, exc, tb))
        QMessageBox.critical(editor, editor.t('error'), text)

    sys.excepthook = handle_exception
    print('SAGE Editor started')
    _log('SAGE Editor started')

    try:
        return app.exec()
    finally:
        _log('SAGE Editor closed')


if __name__ == '__main__':
    main()
