import os
import subprocess
import sys
from pathlib import Path


def launch(path: str) -> None:
    """Launch the engine with the given project or scene path."""
    result = subprocess.run([sys.executable, "-m", "engine", path])
    if result.returncode != 0:
        raise RuntimeError(f"Engine exited with code {result.returncode}")


def list_projects(base: str) -> list[str]:
    """Return all ``.sageproject`` files under ``base``."""
    return [str(p) for p in Path(base).rglob("*.sageproject")]


def create_project(path: str) -> None:
    """Create an empty project file at ``path``."""
    from engine.core.project import Project

    Project(scene={}).save(path)


def run_setup() -> None:
    """Open the SAGE Setup application in a separate process."""
    subprocess.Popen([sys.executable, "-m", "sage_setup"])


def main() -> None:
    try:
        from PyQt6.QtWidgets import (
            QApplication,
            QFileDialog,
            QListWidget,
            QHBoxLayout,
            QPushButton,
            QVBoxLayout,
            QWidget,
            QLineEdit,
            QMessageBox,
        )
    except Exception as exc:  # pragma: no cover - GUI import feedback
        print("PyQt6 is required to run SAGE Launcher", file=sys.stderr)
        raise SystemExit(exc)

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Launcher")
    dir_edit = QLineEdit(os.getcwd())
    browse_btn = QPushButton("Browse")
    project_list = QListWidget()

    def refresh() -> None:
        project_list.clear()
        for proj in list_projects(dir_edit.text()):
            project_list.addItem(proj)

    refresh()

    open_btn = QPushButton("Open")
    create_btn = QPushButton("Create")
    install_btn = QPushButton("Install Engine")

    def choose_dir() -> None:
        path = QFileDialog.getExistingDirectory(
            win, "Select projects directory", dir_edit.text()
        )
        if path:
            dir_edit.setText(path)
            refresh()

    def open_selected() -> None:
        item = project_list.currentItem()
        if not item:
            return
        try:
            launch(item.text())
        except Exception as exc:
            QMessageBox.critical(win, "Launch failed", str(exc))

    def create_dialog() -> None:
        path, _ = QFileDialog.getSaveFileName(
            win,
            "Create project",
            filter="SAGE Project (*.sageproject)",
        )
        if path:
            create_project(path)
            refresh()

    browse_btn.clicked.connect(choose_dir)
    open_btn.clicked.connect(open_selected)
    create_btn.clicked.connect(create_dialog)
    install_btn.clicked.connect(run_setup)

    dir_row = QHBoxLayout()
    dir_row.addWidget(dir_edit)
    dir_row.addWidget(browse_btn)

    btn_row = QHBoxLayout()
    btn_row.addWidget(open_btn)
    btn_row.addWidget(create_btn)
    btn_row.addWidget(install_btn)

    layout = QVBoxLayout(win)
    layout.addLayout(dir_row)
    layout.addWidget(project_list)
    layout.addLayout(btn_row)
    win.show()
    app.exec()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
