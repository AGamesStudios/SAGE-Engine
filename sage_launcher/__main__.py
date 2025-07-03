import os
import subprocess
import sys
from pathlib import Path


def launch(path: str) -> None:
    """Launch the engine with the given project or scene path."""
    command = [sys.executable, "-m", "engine", path]
    subprocess.Popen(command)


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
    from PyQt6.QtWidgets import (
        QApplication,
        QFileDialog,
        QListWidget,
        QHBoxLayout,
        QPushButton,
        QVBoxLayout,
        QWidget,
    )

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Launcher")
    project_list = QListWidget()
    for proj in list_projects(os.getcwd()):
        project_list.addItem(proj)

    open_btn = QPushButton("Open")
    create_btn = QPushButton("Create")
    install_btn = QPushButton("Install Engine")

    def open_selected() -> None:
        item = project_list.currentItem()
        if item:
            launch(item.text())

    def create_dialog() -> None:
        path, _ = QFileDialog.getSaveFileName(
            win,
            "Create project",
            filter="SAGE Project (*.sageproject)",
        )
        if path:
            create_project(path)
            project_list.addItem(path)

    open_btn.clicked.connect(open_selected)
    create_btn.clicked.connect(create_dialog)
    install_btn.clicked.connect(run_setup)

    btn_row = QHBoxLayout()
    btn_row.addWidget(open_btn)
    btn_row.addWidget(create_btn)
    btn_row.addWidget(install_btn)

    layout = QVBoxLayout(win)
    layout.addWidget(project_list)
    layout.addLayout(btn_row)
    win.show()
    app.exec()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
