"""Entry point for SAGE Studio GUI editor."""

import sys
import tkinter as tk
from pathlib import Path

from sage_editor.ui.main_window import build as build_main_window
from sage_editor.core import state

if "sage_editor" not in sys.modules and __name__ == "__main__":
    print("🔧 Запускай из корня проекта: python sage_editor/main.py")


def main() -> None:
    root = tk.Tk()
    root.title("SAGE Studio")
    root.geometry("1280x720")

    state_file = Path.home() / ".sage_editor_state.json"
    state.load_state(state_file)

    main_window = build_main_window(root)
    main_window.pack(expand=True, fill="both")

    def _on_close() -> None:
        if hasattr(main_window, "get_split_positions"):
            main_window.get_split_positions()
        state.save_state(state_file)
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", _on_close)

    root.mainloop()


if __name__ == "__main__":
    main()
