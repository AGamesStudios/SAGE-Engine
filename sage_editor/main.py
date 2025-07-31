"""Entry point for SAGE Studio GUI editor."""

import sys
import tkinter as tk
from tkinter import ttk

from sage_editor.layout.top_bar import build as build_top_bar
from sage_editor.layout.left_bar import build as build_left_bar
from sage_editor.layout.right_bar import build as build_right_bar
from sage_editor.layout.bottom_bar import build as build_bottom_bar
from sage_editor.layout.main_view import build as build_main_view

if "sage_editor" not in sys.modules and __name__ == "__main__":
    print("🔧 Запускай из корня проекта: python sage_editor/main.py")


def main() -> None:
    root = tk.Tk()
    root.title("SAGE Studio")
    root.geometry("1280x720")

    top_bar = build_top_bar(root)
    top_bar.pack(side="top", fill="x")

    left_bar = build_left_bar(root)
    left_bar.pack(side="left", fill="y")

    right_bar = build_right_bar(root)
    right_bar.pack(side="right", fill="y")

    bottom_bar = build_bottom_bar(root)
    bottom_bar.pack(side="bottom", fill="x")

    main_view = build_main_view(root)
    main_view.pack(expand=True, fill="both")

    root.mainloop()


if __name__ == "__main__":
    main()
