from __future__ import annotations

from PyQt6.QtWidgets import QApplication, QStyleFactory
from PyQt6.QtGui import QPalette, QColor


def apply_dark_fusion(app: QApplication) -> None:
    """Apply a dark Fusion theme to *app*."""
    app.setStyle(QStyleFactory.create("Fusion"))

    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(240, 240, 240))
    palette.setColor(QPalette.ColorRole.Base, QColor(42, 42, 42))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(66, 66, 66))
    palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ToolTipText, QColor(240, 240, 240))
    palette.setColor(QPalette.ColorRole.Text, QColor(240, 240, 240))
    palette.setColor(QPalette.ColorRole.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(240, 240, 240))
    palette.setColor(QPalette.ColorRole.BrightText, QColor(255, 0, 0))
    palette.setColor(QPalette.ColorRole.Link, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(240, 240, 240))

    app.setPalette(palette)


def apply_modern_theme(app: QApplication) -> None:
    """Apply an updated dark theme with a slightly larger font."""
    try:
        from PyQt6.QtGui import QFont
    except Exception:  # pragma: no cover - tests may stub Qt modules
        QFont = None

    apply_dark_fusion(app)
    if QFont is not None:
        font: QFont = app.font()
        font.setPointSize(font.pointSize() + 1)
        app.setFont(font)


__all__ = ["apply_dark_fusion", "apply_modern_theme"]
