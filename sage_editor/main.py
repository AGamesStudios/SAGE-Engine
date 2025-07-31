"""Entry point for SAGE Studio GUI editor."""

import sys

if "sage_editor" not in sys.modules and __name__ == "__main__":
    print("🔧 Запускай из корня проекта: python sage_editor/main.py")

from sage_editor.ui.main_window import MainWindow


def main() -> None:
    window = MainWindow()
    window.mainloop()


if __name__ == "__main__":
    main()
