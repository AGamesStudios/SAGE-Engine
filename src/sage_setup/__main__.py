import os
import subprocess
import sys
import select
from pathlib import Path
from typing import Iterable

if __package__ is None or __package__ == "":
    sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
try:
    import tomllib
except ImportError:  # pragma: no cover - Python<3.11
    import tomli as tomllib  # type: ignore[import-not-found,no-redef]

REPO_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_PATH = os.path.join(Path.home(), "sage_engine")
# Default directory to store user projects
DEFAULT_PROJECTS = os.path.join(Path.home(), "SAGE Projects")

# --- simple localisation ---------------------------------------------
try:
    from PyQt6.QtCore import QSettings  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - PyQt6 optional
    QSettings = None

LANG = os.environ.get("SAGE_LANG")


def load_language() -> str:
    if QSettings is None:
        return LANG or "en"
    settings = QSettings("AGStudios", "sage_setup")
    return settings.value("language", LANG or "en")


def save_language(lang: str) -> None:
    if QSettings is None:
        return
    settings = QSettings("AGStudios", "sage_setup")
    settings.setValue("language", lang)


STRINGS: dict[str, dict[str, str]] = {
    "en": {
        "browse": "Browse",
        "install": "Install",
        "install_loc": "Install location:",
        "extras": "Extras:",
        "version": "Version:",
    },
    "ru": {
        "browse": "Обзор",
        "install": "Установить",
        "install_loc": "Путь установки:",
        "extras": "Дополнения:",
        "version": "Версия:",
    },
}


def installed_versions(path: str = DEFAULT_PATH) -> list[str]:
    """Return installed engine versions under ``path``."""
    if not os.path.isdir(path):
        return []
    return sorted([d.name for d in Path(path).iterdir() if d.is_dir()])


def tr(key: str) -> str:
    lang = load_language()
    return STRINGS.get(lang, STRINGS["en"]).get(key, key)


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


def _parse_progress(line: str) -> int | None:
    import re

    m = re.search(r"(\d{1,3})%", line)
    if m:
        return int(m.group(1))
    return None


def install_iter(
    target: str | None = DEFAULT_PATH,
    extras: str | None = None,
    version: str | None = None,
    *,
    launcher_only: bool = False,
) -> Iterable[tuple[str, int | None]]:
    """Yield pip output and progress while installing."""
    if launcher_only:
        package = "sage-launcher"
    else:
        package = REPO_ROOT if version is None else f"sage-engine=={version}"
        if extras:
            package += f"[{extras}]"
    command = [
        sys.executable,
        "-m",
        "pip",
        "install",
        package,
        "--progress-bar",
        "off",
        "--verbose",
    ]
    if target is not None and not launcher_only:
        if version is not None:
            target = os.path.join(target, version)
        command += ["--target", target]
    proc = subprocess.Popen(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
    )
    output = ""
    assert proc.stdout is not None and proc.stderr is not None
    use_select = True
    try:
        proc.stdout.fileno()
        proc.stderr.fileno()
    except Exception:
        use_select = False

    if use_select:
        streams = [proc.stdout, proc.stderr]
        while streams:
            ready, _, _ = select.select(streams, [], [], 0.1)
            for r in ready:
                line = r.readline()
                if line:
                    output += line
                    yield line, _parse_progress(line)
                elif proc.poll() is not None:
                    streams.remove(r)
    else:
        out = proc.stdout.read()
        err = proc.stderr.read()
        for line in (out + err).splitlines(True):
            output += line
            yield line, _parse_progress(line)
    if proc.returncode != 0:
        raise RuntimeError(output)


def install(
    target: str | None = DEFAULT_PATH,
    extras: str | None = None,
    version: str | None = None,
    *,
    launcher_only: bool = False,
) -> str:
    """Run ``pip install`` for the engine in ``target`` with optional extras."""
    output = "".join(
        line for line, _ in install_iter(target, extras, version, launcher_only=launcher_only)
    )
    if not launcher_only:
        os.makedirs(DEFAULT_PROJECTS, exist_ok=True)
    return output




def run_install_dialog(path: str | None, extras: str | None, version: str | None, win, *, launcher_only: bool = False) -> str:
    """Run install and display progress output."""
    from PyQt6.QtWidgets import (
        QApplication,
        QDialog,
        QPlainTextEdit,
        QProgressBar,
        QVBoxLayout,
        QMessageBox,
    )  # type: ignore[import-not-found]
    from PyQt6.QtCore import Qt  # type: ignore[import-not-found]

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
    determinate = False
    try:
        for line, prog in install_iter(path, extras, version, launcher_only=launcher_only):
            output.appendPlainText(line.rstrip())
            if prog is not None:
                if not determinate:
                    bar.setRange(0, 100)
                    determinate = True
                bar.setValue(prog)
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
    import argparse

    parser = argparse.ArgumentParser()
    parser.add_argument("--launcher-only", action="store_true")
    args, _ = parser.parse_known_args()

    if args.launcher_only:
        print(install(launcher_only=True))
        return

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
        )  # type: ignore[import-not-found]
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
    browse_btn = QPushButton(tr("browse"))
    version_edit = QLineEdit()
    version_edit.setPlaceholderText("dev")
    extras_boxes = {name: QCheckBox(name) for name in available_extras()}

    def browse() -> None:
        path = QFileDialog.getExistingDirectory(win, "Select install location")
        if path:
            path_edit.setText(path)

    def run_install() -> None:
        selected = [n for n, box in extras_boxes.items() if box.isChecked()]
        extras = ",".join(selected) or None
        run_install_dialog(
            path_edit.text() or None,
            extras,
            version_edit.text() or None,
            win,
        )

    browse_btn.clicked.connect(browse)
    install_btn = QPushButton(tr("install"))
    install_btn.clicked.connect(run_install)

    form = QFormLayout()
    path_row = QHBoxLayout()
    path_row.addWidget(path_edit)
    path_row.addWidget(browse_btn)
    form.addRow(tr("install_loc"), path_row)
    form.addRow(tr("version"), version_edit)
    extras_layout = QGridLayout()
    for i, box in enumerate(extras_boxes.values()):
        row = i // 6
        col = i % 6
        extras_layout.addWidget(box, row, col)
    form.addRow(tr("extras"), extras_layout)

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
