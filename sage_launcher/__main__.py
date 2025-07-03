import os
import subprocess
import sys
import webbrowser
from pathlib import Path

DOC_URL = "https://github.com/AGamesStudios/SAGE-Engine"

try:
    from PyQt6.QtCore import QSettings  # type: ignore
except Exception:  # pragma: no cover - PyQt6 optional
    QSettings = None


def launch(path: str) -> subprocess.Popen[str]:
    """Launch the engine with the given project or scene path and return the process."""
    return subprocess.Popen(
        [sys.executable, "-m", "engine", path], start_new_session=True
    )


def list_projects(base: str, *, recursive: bool = True) -> list[str]:
    """Return ``.sageproject`` files under ``base``."""
    if recursive:
        paths = Path(base).rglob("*.sageproject")
    else:
        paths = Path(base).glob("*.sageproject")
    return [str(p) for p in paths]


def load_last_dir() -> str:
    """Return last directory stored in ``QSettings`` or ``os.getcwd``."""
    if QSettings is None:
        return os.getcwd()
    settings = QSettings("AGStudios", "sage_launcher")
    return settings.value("last_dir", os.getcwd())


def save_last_dir(path: str) -> None:
    """Persist ``path`` using ``QSettings``."""
    if QSettings is None:
        return
    settings = QSettings("AGStudios", "sage_launcher")
    dirs = settings.value("recent_dirs", [], list) or []
    if path in dirs:
        dirs.remove(path)
    dirs.insert(0, path)
    settings.setValue("recent_dirs", dirs[:5])
    settings.setValue("last_dir", path)


def load_recent_dirs() -> list[str]:
    """Return a list of recently used directories."""
    if QSettings is None:
        return []
    settings = QSettings("AGStudios", "sage_launcher")
    return settings.value("recent_dirs", [], list) or []


def create_project(path: str) -> None:
    """Create a minimal project with a camera."""
    from engine.core.project import Project

    scene = {
        "objects": [
            {"type": "camera", "width": 640, "height": 480, "active": True}
        ]
    }
    Project(scene=scene).save(path)


def run_setup() -> None:
    """Open the SAGE Setup application in a separate process."""
    subprocess.Popen([sys.executable, "-m", "sage_setup"])


def open_docs() -> None:
    """Open the SAGE documentation URL."""
    webbrowser.open(DOC_URL)


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
            QTabWidget,
            QLabel,
        )
    except Exception as exc:  # pragma: no cover - GUI import feedback
        print("PyQt6 is required to run SAGE Launcher", file=sys.stderr)
        raise SystemExit(exc)

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Launcher")
    win.setFixedSize(500, 300)
    tabs = QTabWidget()

    # --- Projects tab ---
    project_tab = QWidget()
    dir_edit = QLineEdit(load_last_dir())
    browse_btn = QPushButton("Browse")
    project_list = QListWidget()

    def refresh() -> None:
        project_list.clear()
        for proj in list_projects(dir_edit.text()):
            project_list.addItem(proj)

    refresh()

    open_btn = QPushButton("Open")
    create_btn = QPushButton("Create")
    install_btn = QPushButton("Install/Update Engine")

    def choose_dir() -> None:
        path = QFileDialog.getExistingDirectory(
            win, "Select projects directory", dir_edit.text()
        )
        if path:
            dir_edit.setText(path)
            save_last_dir(path)
            refresh()

    def open_selected() -> None:
        item = project_list.currentItem()
        if not item:
            return
        try:
            proc = launch(item.text())
            def wait_and_report() -> None:
                rc = proc.wait()
                if rc != 0:
                    QMessageBox.warning(
                        win,
                        "Game exited",
                        f"Process returned code {rc}",
                    )

            import threading

            threading.Thread(target=wait_and_report, daemon=True).start()
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

    proj_layout = QVBoxLayout(project_tab)
    proj_layout.addLayout(dir_row)
    proj_layout.addWidget(project_list)
    proj_layout.addLayout(btn_row)

    # --- Engine tab ---
    engine_tab = QWidget()
    update_btn = QPushButton("Install/Update Engine")
    update_btn.clicked.connect(run_setup)
    engine_layout = QVBoxLayout(engine_tab)
    engine_layout.addWidget(update_btn)
    engine_layout.addStretch()

    # --- Info tab ---
    info_tab = QWidget()
    docs_btn = QPushButton("Open Documentation")
    docs_btn.clicked.connect(open_docs)
    info_layout = QVBoxLayout(info_tab)
    info_layout.addWidget(QLabel("Documentation and tools"))
    info_layout.addWidget(docs_btn)
    info_layout.addStretch()

    tabs.addTab(project_tab, "Projects")
    tabs.addTab(engine_tab, "Engine")
    tabs.addTab(info_tab, "Info")

    layout = QVBoxLayout(win)
    layout.addWidget(tabs)
    win.show()
    fg = win.frameGeometry()
    fg.moveCenter(app.primaryScreen().availableGeometry().center())
    win.move(fg.topLeft())
    app.exec()
    save_last_dir(dir_edit.text())


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
