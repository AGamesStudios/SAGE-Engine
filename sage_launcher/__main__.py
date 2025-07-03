import subprocess
import sys
import tkinter as tk
from tkinter import ttk, filedialog


def launch(path: str) -> None:
    """Launch the engine with the given project or scene path."""
    command = [sys.executable, "-m", "engine", path]
    subprocess.Popen(command)


def main() -> None:
    root = tk.Tk()
    root.title("SAGE Launcher")

    def pick_and_launch() -> None:
        file = filedialog.askopenfilename(
            title="Select project or scene",
            filetypes=[("SAGE files", "*.sageproject *.sagescene")],
        )
        if file:
            launch(file)

    ttk.Button(root, text="Open Project", command=pick_and_launch).pack(
        padx=20, pady=20
    )
    root.mainloop()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
