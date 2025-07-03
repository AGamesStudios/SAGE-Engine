import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_PATH = os.path.join(Path.home(), "sage_engine")


def install(target: str | None = DEFAULT_PATH, extras: str | None = None) -> None:
    """Run ``pip install`` for the engine in ``target`` with optional extras."""
    command = [sys.executable, "-m", "pip", "install", REPO_ROOT]
    if extras:
        command[-1] += f"[{extras}]"
    if target is not None:
        command += ["--target", target]
    subprocess.check_call(command)


def main() -> None:
    from PyQt6.QtCore import Qt
    from PyQt6.QtWidgets import (
        QApplication,
        QFileDialog,
        QFormLayout,
        QHBoxLayout,
        QLineEdit,
        QMessageBox,
        QPushButton,
        QVBoxLayout,
        QWidget,
        QCheckBox,
        QProgressDialog,
    )

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Setup")
    path_edit = QLineEdit(DEFAULT_PATH)
    browse_btn = QPushButton("Browse")
    extras_boxes = {
        name: QCheckBox(name)
        for name in ("opengl", "sdl", "audio", "qt", "sdk")
    }

    def browse() -> None:
        path = QFileDialog.getExistingDirectory(win, "Select install location")
        if path:
            path_edit.setText(path)

    def run_install() -> None:
        selected = [n for n, box in extras_boxes.items() if box.isChecked()]
        extras = ",".join(selected) or None
        progress = QProgressDialog("Installing...", "", 0, 0, win)
        progress.setWindowTitle("SAGE Setup")
        progress.setWindowModality(Qt.WindowModality.WindowModal)
        progress.show()
        QApplication.processEvents()
        try:
            install(path_edit.text() or None, extras)
        except Exception as exc:  # pragma: no cover - GUI feedback only
            progress.close()
            QMessageBox.critical(win, "SAGE Setup", f"Install failed: {exc}")
        else:
            progress.close()
            QMessageBox.information(win, "SAGE Setup", "Installation complete")

    browse_btn.clicked.connect(browse)
    install_btn = QPushButton("Install")
    install_btn.clicked.connect(run_install)

    form = QFormLayout()
    path_row = QHBoxLayout()
    path_row.addWidget(path_edit)
    path_row.addWidget(browse_btn)
    form.addRow("Install location:", path_row)
    extras_layout = QVBoxLayout()
    for box in extras_boxes.values():
        extras_layout.addWidget(box)
    form.addRow("Extras:", extras_layout)

    layout = QVBoxLayout(win)
    layout.addLayout(form)
    layout.addWidget(install_btn)

    win.show()
    app.exec()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
