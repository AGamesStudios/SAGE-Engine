import os
import subprocess
import sys

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


def install(target: str | None = None, extras: str | None = None) -> None:
    """Run ``pip install`` for the engine in ``target`` with optional extras."""
    command = [sys.executable, "-m", "pip", "install", REPO_ROOT]
    if extras:
        command[-1] += f"[{extras}]"
    if target:
        command += ["--target", target]
    subprocess.check_call(command)


def main() -> None:
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
    )

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Setup")
    path_edit = QLineEdit()
    browse_btn = QPushButton("Browse")
    extras_boxes = {name: QCheckBox(name) for name in ("opengl", "sdl", "audio")}

    def browse() -> None:
        path = QFileDialog.getExistingDirectory(win, "Select install location")
        if path:
            path_edit.setText(path)

    def run_install() -> None:
        selected = [n for n, box in extras_boxes.items() if box.isChecked()]
        extras = ",".join(selected) or None
        try:
            install(path_edit.text() or None, extras)
        except Exception as exc:  # pragma: no cover - GUI feedback only
            QMessageBox.critical(win, "SAGE Setup", f"Install failed: {exc}")
        else:
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
