import os
import subprocess
import sys
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

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
    root = tk.Tk()
    root.title("SAGE Setup")
    path_var = tk.StringVar()
    extras_var = tk.StringVar()

    def browse() -> None:
        path = filedialog.askdirectory(title="Select install location")
        if path:
            path_var.set(path)

    def run_install() -> None:
        try:
            install(path_var.get() or None, extras_var.get() or None)
        except Exception as exc:  # pragma: no cover - GUI feedback only
            messagebox.showerror("SAGE Setup", f"Install failed: {exc}")
        else:
            messagebox.showinfo("SAGE Setup", "Installation complete")

    frame = ttk.Frame(root, padding=10)
    frame.pack(fill="both", expand=True)
    ttk.Label(frame, text="Install location:").grid(row=0, column=0, sticky="w")
    ttk.Entry(frame, textvariable=path_var, width=40).grid(row=0, column=1)
    ttk.Button(frame, text="Browse", command=browse).grid(row=0, column=2, padx=5)

    ttk.Label(frame, text="Extras (comma separated):").grid(row=1, column=0, sticky="w")
    ttk.Entry(frame, textvariable=extras_var, width=40).grid(row=1, column=1, columnspan=2)

    ttk.Button(frame, text="Install", command=run_install).grid(row=2, column=0, columnspan=3, pady=10)
    root.mainloop()


if __name__ == "__main__":  # pragma: no cover - CLI entry point
    main()
