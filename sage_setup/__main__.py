import os
import subprocess
import sys
from pathlib import Path
from typing import Iterable
try:
    import tomllib
except ImportError:  # pragma: no cover - Python<3.11
    import tomli as tomllib

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_PATH = os.path.join(Path.home(), "sage_engine")


def available_extras() -> list[str]:
    """Return extras defined for the package."""
    pyproject = os.path.join(REPO_ROOT, "pyproject.toml")
    if os.path.exists(pyproject):
        with open(pyproject, "rb") as f:
            data = tomllib.load(f)
        deps = data.get("project", {}).get("optional-dependencies", {})
        return sorted(deps.keys())
    # fall back to installed metadata
    import importlib.metadata

    try:
        meta = importlib.metadata.metadata("sage-engine")
    except importlib.metadata.PackageNotFoundError:
        return []
    return sorted(meta.get_all("Provides-Extra") or [])


def install_iter(target: str | None = DEFAULT_PATH, extras: str | None = None) -> Iterable[str]:
    """Yield pip output while installing the engine."""
    command = [sys.executable, "-m", "pip", "install", REPO_ROOT]
    if extras:
        command[-1] += f"[{extras}]"
    if target is not None:
        command += ["--target", target]
    proc = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    output = ""
    assert proc.stdout is not None
    for line in proc.stdout:
        output += line
        yield line
    proc.wait()
    if proc.returncode != 0:
        raise RuntimeError(output)


def install(target: str | None = DEFAULT_PATH, extras: str | None = None) -> str:
    """Run ``pip install`` for the engine in ``target`` with optional extras."""
    output = "".join(install_iter(target, extras))
    return output


def run_install_dialog(path: str | None, extras: str | None, win) -> str:
    """Run install and display progress output."""
    from PyQt6.QtWidgets import (
        QApplication,
        QDialog,
        QPlainTextEdit,
        QProgressBar,
        QVBoxLayout,
        QMessageBox,
    )
    from PyQt6.QtCore import Qt

    progress = QDialog(win)
    progress.setWindowTitle("SAGE Setup")
    progress.setWindowModality(Qt.WindowModality.WindowModal)
    layout = QVBoxLayout(progress)
    output = QPlainTextEdit()
    output.setReadOnly(True)
    bar = QProgressBar()
    bar.setRange(0, 0)
    layout.addWidget(output)
    layout.addWidget(bar)
    progress.setFixedSize(500, 300)
    pg = progress.frameGeometry()
    pg.moveCenter(win.frameGeometry().center())
    progress.move(pg.topLeft())
    progress.show()
    QApplication.processEvents()
    try:
        for line in install_iter(path, extras):
            output.appendPlainText(line.rstrip())
            QApplication.processEvents()
    except Exception as exc:  # pragma: no cover - GUI feedback only
        output.appendPlainText(str(exc))
        bar.setRange(0, 1)
        bar.setValue(1)
        progress.close()
        QMessageBox.critical(win, "SAGE Setup", f"Install failed:\n{exc}")
    else:
        bar.setRange(0, 1)
        bar.setValue(1)
        progress.close()
        QMessageBox.information(win, "SAGE Setup", "Installation complete")
    return output.toPlainText()


def main() -> None:
    try:
        from PyQt6.QtWidgets import (
            QApplication,
            QFileDialog,
            QFormLayout,
            QHBoxLayout,
            QLineEdit,
            QPushButton,
            QVBoxLayout,
            QWidget,
            QCheckBox,
            QGridLayout,
        )
    except Exception as exc:  # pragma: no cover - GUI import feedback
        print("PyQt6 is required to run SAGE Setup", file=sys.stderr)
        raise SystemExit(exc)

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Setup")
    win.setFixedSize(500, 250)
    path_edit = QLineEdit()
    path_edit.setPlaceholderText(DEFAULT_PATH)
    browse_btn = QPushButton("Browse")
    extras_boxes = {name: QCheckBox(name) for name in available_extras()}

    def browse() -> None:
        path = QFileDialog.getExistingDirectory(win, "Select install location")
        if path:
            path_edit.setText(path)

    def run_install() -> None:
        selected = [n for n, box in extras_boxes.items() if box.isChecked()]
        extras = ",".join(selected) or None
        run_install_dialog(path_edit.text() or None, extras, win)

    browse_btn.clicked.connect(browse)
    install_btn = QPushButton("Install")
    install_btn.clicked.connect(run_install)

    form = QFormLayout()
    path_row = QHBoxLayout()
    path_row.addWidget(path_edit)
    path_row.addWidget(browse_btn)
    form.addRow("Install location:", path_row)
    extras_layout = QGridLayout()
    for i, box in enumerate(extras_boxes.values()):
        row = i // 6
        col = i % 6
        extras_layout.addWidget(box, row, col)
    form.addRow("Extras:", extras_layout)

    layout = QVBoxLayout(win)
    layout.addLayout(form)
    layout.addWidget(install_btn)

    win.show()
    fg = win.frameGeometry()
    center = app.primaryScreen().availableGeometry().center()
    fg.moveCenter(center)
    win.move(fg.topLeft())
    app.exec()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
