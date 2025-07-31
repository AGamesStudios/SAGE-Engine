"""Entry point for SAGE Studio GUI editor."""

import sys
import tkinter as tk

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

    def run_preview() -> None:
        print("Running preview...")

    def save_project() -> None:
        print("Saving project...")

    def open_settings() -> None:
        win = tk.Toplevel(root)
        win.title("Настройки проекта")
        tk.Label(win, text="Настройки проекта").pack(padx=20, pady=20)

    root.rowconfigure(1, weight=1)
    root.columnconfigure(0, weight=1)

    top_bar = build_top_bar(root, run_cb=run_preview, save_cb=save_project, settings_cb=open_settings)
    top_bar.grid(row=0, column=0, sticky="ew")

    main_area = tk.Frame(root, bg="#1e1e1e")
    main_area.grid(row=1, column=0, sticky="nsew")
    main_area.columnconfigure(0, weight=0)
    main_area.columnconfigure(1, weight=1)
    main_area.columnconfigure(2, weight=0)
    main_area.rowconfigure(0, weight=1)

    left_bar = build_left_bar(main_area)
    left_bar.grid(row=0, column=0, sticky="ns", padx=(0, 2), pady=2)

    center = build_main_view(main_area)
    center.grid(row=0, column=1, sticky="nsew", padx=2, pady=2)

    right_bar = build_right_bar(main_area)
    right_bar.grid(row=0, column=2, sticky="ns", padx=(2, 0), pady=2)

    bottom_bar = build_bottom_bar(root)
    bottom_bar.grid(row=2, column=0, sticky="ew")

    root.mainloop()


if __name__ == "__main__":
    main()
