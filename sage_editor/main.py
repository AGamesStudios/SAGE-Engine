"""Entry point for SAGE Studio GUI editor."""

import sys
import tkinter as tk

from sage_editor.ui.main_window import build as build_main_window

if "sage_editor" not in sys.modules and __name__ == "__main__":
    print("🔧 Запускай из корня проекта: python sage_editor/main.py")


def main() -> None:
    root = tk.Tk()
    root.title("SAGE Studio")
    root.geometry("1280x720")

    main_window = build_main_window(root)
    main_window.pack(expand=True, fill="both")

    root.mainloop()


if __name__ == "__main__":
    main()
