import os
import subprocess
import sys
from pathlib import Path
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


def install(target: str | None = DEFAULT_PATH, extras: str | None = None) -> str:
    """Run ``pip install`` for the engine in ``target`` with optional extras."""
    command = [sys.executable, "-m", "pip", "install", REPO_ROOT]
    if extras:
        command[-1] += f"[{extras}]"
    if target is not None:
        command += ["--target", target]
    result = subprocess.run(command, capture_output=True, text=True)
    output = (result.stdout or "") + (result.stderr or "")
    if result.returncode != 0:
        raise RuntimeError(output)
    return output


def run_install_dialog(path: str | None, extras: str | None, win) -> str:
    """Run install and display progress output."""
    from PyQt6.QtWidgets import QApplication, QProgressDialog, QMessageBox
    from PyQt6.QtCore import Qt

    progress = QProgressDialog("Installing...", "", 0, 0, win)
    progress.setWindowTitle("SAGE Setup")
    progress.setWindowModality(Qt.WindowModality.WindowModal)
    progress.show()
    QApplication.processEvents()
    try:
        output = install(path, extras)
        if output:
            progress.setLabelText(progress.labelText() + "\n" + output)
    except Exception as exc:  # pragma: no cover - GUI feedback only
        progress.setLabelText(progress.labelText() + "\n" + str(exc))
        progress.close()
        QMessageBox.critical(win, "SAGE Setup", f"Install failed:\n{exc}")
    else:
        progress.close()
        QMessageBox.information(win, "SAGE Setup", "Installation complete")
    return progress.labelText()


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
        )
    except Exception as exc:  # pragma: no cover - GUI import feedback
        print("PyQt6 is required to run SAGE Setup", file=sys.stderr)
        raise SystemExit(exc)

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Setup")
    path_edit = QLineEdit(DEFAULT_PATH)
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
