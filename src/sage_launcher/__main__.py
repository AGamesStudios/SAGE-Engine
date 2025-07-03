import os
import subprocess
import sys
import webbrowser
from pathlib import Path

if __package__ is None or __package__ == "":
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

try:
    from sage_setup import installed_versions, DEFAULT_PATH, DEFAULT_PROJECTS
except Exception as exc:  # pragma: no cover - missing dependency
    print(
        "SAGE Setup is required to run the launcher.\n"
        "Install the engine with `python -m pip install .` or `python -m sage_setup`.",
        file=sys.stderr,
    )
    raise SystemExit(exc)

DOC_URL = "https://github.com/AGamesStudios/SAGE-Engine"

# --- simple localisation ---------------------------------------------
LANG = os.environ.get("SAGE_LANG")

def load_language() -> str:
    if QSettings is None:
        return LANG or "en"
    settings = QSettings("AGStudios", "sage_launcher")
    return settings.value("language", LANG or "en")


def save_language(lang: str) -> None:
    if QSettings is None:
        return
    settings = QSettings("AGStudios", "sage_launcher")
    settings.setValue("language", lang)


STRINGS: dict[str, dict[str, str]] = {
    "en": {
        "browse": "Browse",
        "open": "Open",
        "create": "Create",
        "install": "Install Engine",
        "projects": "Projects",
        "engine": "Engine",
        "info": "Info",
        "docs": "Open Documentation",
        "docs_label": "Documentation and tools",
        "language": "Language:",
    },
    "ru": {
        "browse": "Обзор",
        "open": "Открыть",
        "create": "Создать",
        "install": "Установить Движок",
        "projects": "Проекты",
        "engine": "Движок",
        "info": "Инфо",
        "docs": "Открыть Документацию",
        "docs_label": "Документация и инструменты",
        "language": "Язык:",
    },
}


def tr(key: str) -> str:
    lang = load_language()
    return STRINGS.get(lang, STRINGS["en"]).get(key, key)

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
        return DEFAULT_PROJECTS
    settings = QSettings("AGStudios", "sage_launcher")
    return settings.value("last_dir", DEFAULT_PROJECTS)


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


TEMPLATE_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", "..", "examples", "templates")
)


def create_project(path: str, version: str = "dev", template: str = "blank") -> None:
    """Create a new project from ``template`` and set the engine version."""
    import json
    import shutil

    tmpl_dir = os.path.join(TEMPLATE_DIR, template)
    shutil.copytree(tmpl_dir, os.path.dirname(path), dirs_exist_ok=True)
    src_proj = os.path.join(os.path.dirname(path), "project.sageproject")
    os.replace(src_proj, path)
    data = json.loads(Path(path).read_text())
    data.setdefault("metadata", {})["engine_version"] = version
    Path(path).write_text(json.dumps(data, indent=2))


def run_setup(*args: str) -> None:
    """Open the SAGE Setup application in a separate process."""
    subprocess.Popen([sys.executable, "-m", "sage_setup", *args])


def open_docs() -> None:
    """Open local documentation or fallback to the online URL."""
    local = os.environ.get("SAGE_DOC_PATH")
    if local and os.path.exists(local):
        if not local.startswith("file://"):
            local = "file://" + local
        webbrowser.open(local)
    else:
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
    browse_btn = QPushButton(tr("browse"))
    project_list = QListWidget()

    def refresh() -> None:
        project_list.clear()
        for proj in list_projects(dir_edit.text()):
            project_list.addItem(proj)

    refresh()

    open_btn = QPushButton(tr("open"))
    create_btn = QPushButton(tr("create"))
    install_btn = QPushButton(tr("install"))

    def choose_dir() -> None:
        path = QFileDialog.getExistingDirectory(
            win,
            tr("browse"),
            dir_edit.text(),
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
            tr("create"),
            filter="SAGE Project (*.sageproject)",
        )
        if not path:
            return
        versions = installed_versions(DEFAULT_PATH)
        if not versions:
            version = "dev"
        else:
            from PyQt6.QtWidgets import QInputDialog

            version, ok = QInputDialog.getItem(
                win, tr("create"), "Version", versions, 0, False
            )
            if not ok:
                return
        from PyQt6.QtWidgets import QInputDialog
        templates = [d.name for d in Path(TEMPLATE_DIR).iterdir() if d.is_dir()]
        tmpl, ok = QInputDialog.getItem(
            win, tr("create"), "Template", templates, 0, False
        )
        if not ok:
            return
        create_project(path, version, tmpl)
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
    update_btn = QPushButton(tr("install"))
    update_btn.clicked.connect(lambda: run_setup())
    engine_layout = QVBoxLayout(engine_tab)
    engine_layout.addWidget(update_btn)
    engine_layout.addStretch()

    # --- Info tab ---
    info_tab = QWidget()
    docs_btn = QPushButton(tr("docs"))
    docs_btn.clicked.connect(open_docs)
    lang_label = QLabel(tr("language"))
    lang_box = QListWidget()
    lang_box.addItems(["English", "Русский"])
    current = 0 if load_language() == "en" else 1
    lang_box.setCurrentRow(current)

    def set_lang() -> None:
        save_language("en" if lang_box.currentRow() == 0 else "ru")

    lang_box.currentRowChanged.connect(lambda _: set_lang())

    info_layout = QVBoxLayout(info_tab)
    info_layout.addWidget(QLabel(tr("docs_label")))
    info_layout.addWidget(docs_btn)
    info_layout.addWidget(lang_label)
    info_layout.addWidget(lang_box)
    info_layout.addStretch()

    tabs.addTab(project_tab, tr("projects"))
    tabs.addTab(engine_tab, tr("engine"))
    tabs.addTab(info_tab, tr("info"))

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
