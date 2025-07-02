from __future__ import annotations

from PyQt6.QtWidgets import QApplication, QStyleFactory, QWidget
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
    """Apply an updated dark Fusion theme with larger fonts and widget styling."""
    try:
        from PyQt6.QtGui import QFont
    except Exception:  # pragma: no cover - tests may stub Qt modules
        QFont = None

    apply_dark_fusion(app)
    if QFont is not None:
        font: QFont = app.font()
        font.setPointSize(font.pointSize() + 1)
        app.setFont(font)

    # subtle widget borders and headings
    app.setStyleSheet(
        """
        QGroupBox { font-weight: bold; border: 1px solid #555; margin-top: 8px; }
        QGroupBox::title { subcontrol-origin: margin; left: 4px; top: -2px; }
        QPlainTextEdit { background: #1e1e1e; color: #dcdcdc; }
        QListWidget { background: #2b2b2b; color: #ddd; }
        QListWidget::item:selected { background: #444; }
        QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox {
            background: #2b2b2b; color: #ddd; border: 1px solid #555;
        }
        QSlider::groove:horizontal { background: #444; height: 6px; }
        QSlider::handle:horizontal {
            background: #dcdcdc; border: 1px solid #555; width: 10px; margin: -4px 0;
        }
        QPushButton { background: #3c3c3c; color: #ddd; border: 1px solid #555; }
        """
    )


def fit_to_screen(window: QWidget, scale: float = 0.8) -> None:
    """Resize *window* to a fraction of the primary screen."""
    screen = QApplication.primaryScreen()
    if screen is None:
        return
    rect = screen.availableGeometry()
    width = int(rect.width() * scale)
    height = int(rect.height() * scale)
    window.resize(width, height)


__all__ = [
    "apply_dark_fusion",
    "apply_modern_theme",
    "fit_to_screen",
]
