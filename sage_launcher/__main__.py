import subprocess
import sys


def launch(path: str) -> None:
    """Launch the engine with the given project or scene path."""
    command = [sys.executable, "-m", "engine", path]
    subprocess.Popen(command)


def main() -> None:
    from PyQt6.QtWidgets import (
        QApplication,
        QFileDialog,
        QPushButton,
        QVBoxLayout,
        QWidget,
    )

    app = QApplication(sys.argv)
    app.setStyle("Fusion")

    win = QWidget()
    win.setWindowTitle("SAGE Launcher")
    open_btn = QPushButton("Open Project")

    def pick_and_launch() -> None:
        file, _ = QFileDialog.getOpenFileName(
            win,
            "Select project or scene",
            filter="SAGE files (*.sageproject *.sagescene)",
        )
        if file:
            launch(file)

    open_btn.clicked.connect(pick_and_launch)
    layout = QVBoxLayout(win)
    layout.addWidget(open_btn)
    win.show()
    app.exec()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
