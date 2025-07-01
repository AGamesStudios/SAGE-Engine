"""Standalone application entry point and project manager."""

from __future__ import annotations

import sys
import os
import traceback

from PyQt6.QtWidgets import (
    QApplication, QDialog, QLabel, QPushButton, QMessageBox
)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QPalette, QColor, QFont
from .icons import load_icon, app_icon

DARK_STYLE = """
QToolBar { icon-size: 24px; spacing: 6px; }
QDockWidget::title { padding: 4px; background: #333; color: #ddd; }
QGroupBox { margin-top: 8px; }
QGroupBox::title { subcontrol-origin: margin; left: 4px; padding: 0 2px; }
QPushButton { padding: 4px 8px; }
QLineEdit { padding: 2px 4px; }
QListWidget { background: #222; }
QDockWidget { font-size: 11px; }
QToolTip {
    color: #eee;
    background: #444;
    border: 1px solid #666;
    padding: 4px;
    font-size: 12px;
}
"""


def apply_palette() -> None:
    """Apply the dark palette to the QApplication."""
    app = QApplication.instance()
    if app is None:
        return
    app.setStyle("Fusion")
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Base, QColor(35, 35, 35))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ToolTipText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Text, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(0, 0, 0))
    app.setPalette(palette)
    app.setStyleSheet(DARK_STYLE)

def apply_light_palette() -> None:
    """Apply a light Fusion palette to the QApplication."""
    app = QApplication.instance()
    if app is None:
        return
    app.setStyle("Fusion")
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(255, 255, 255))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(0, 0, 0))
    palette.setColor(QPalette.ColorRole.Base, QColor(255, 255, 255))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(240, 240, 240))
    palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(255, 255, 220))
    palette.setColor(QPalette.ColorRole.ToolTipText, QColor(0, 0, 0))
    palette.setColor(QPalette.ColorRole.Text, QColor(0, 0, 0))
    palette.setColor(QPalette.ColorRole.Button, QColor(240, 240, 240))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(0, 0, 0))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(0, 120, 215))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(255, 255, 255))
    app.setPalette(palette)
    app.setStyleSheet("")



def apply_default_palette() -> None:
    """Restore the default Qt palette and style."""
    app = QApplication.instance()
    if app is None:
        return
    app.setStyle("Fusion")
    app.setPalette(QApplication.style().standardPalette())
    app.setStyleSheet("")

from .editor import Editor, save_recent, load_recent, _log, logger
from .project_hub import ProjectHub
ProjectManager = ProjectHub




def main(argv=None):
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    app.setWindowIcon(app_icon())
    from .editor import load_settings
    from .icons import set_icon_theme
    settings = load_settings()
    theme = settings.get("theme", "dark")
    if theme == "light":
        set_icon_theme("black")
        apply_light_palette()
    else:
        set_icon_theme("white")
        apply_palette()
    font = QFont()
    font.setPointSize(font.pointSize() + 3)
    app.setFont(font)
    editor = Editor(autoshow=False)
    editor.font_size = font.pointSize()
    pm = ProjectManager(editor)
    if pm.exec() != QDialog.DialogCode.Accepted:
        app.quit()
        sys.exit(0)
        return 0
    editor.showMaximized()

    orig_out = sys.stdout
    orig_err = sys.stderr

    class _Stream:
        """Mirror writes to the editor console and an optional original stream."""

        def __init__(self, dock, orig):
            self.dock = dock
            self.orig = orig

        def write(self, text):
            if text.strip():
                self.dock.write(text.rstrip())
            if self.orig is not None:
                self.orig.write(text)

        def flush(self):
            if self.orig is not None:
                self.orig.flush()

    sys.stdout = _Stream(editor.console_dock, orig_out)
    sys.stderr = _Stream(editor.console_dock, orig_err)
    # ensure loggers send output to the editor console
    from engine.utils.log import set_stream as set_engine_stream
    from .editor import set_stream as set_editor_stream
    set_engine_stream(sys.stderr)
    set_editor_stream(sys.stderr)

    def handle_exception(exc_type, exc, tb):
        text = ''.join(traceback.format_exception(exc_type, exc, tb))
        editor.console_dock.write(text)
        if orig_err is not None:
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
