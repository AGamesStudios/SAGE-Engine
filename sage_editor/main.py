"""Entry point for SAGE Studio GUI editor."""

import os
import sys
import customtkinter as ctk

if "sage_editor" not in sys.modules and __name__ == "__main__":
    print("🔧 Запускай из корня проекта: python sage_editor/main.py")

ctk.set_appearance_mode("dark")
ctk.set_default_color_theme(os.path.join(os.path.dirname(__file__), "themes", "sage_theme.json"))

from sage_editor.ui.main_window import MainWindow


def main() -> None:
    window = MainWindow()
    window.mainloop()


if __name__ == "__main__":
    main()
