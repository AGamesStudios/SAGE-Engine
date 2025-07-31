"""Entry point for SAGE Studio GUI editor."""

from .ui.main_window import MainWindow


def main() -> None:
    window = MainWindow()
    window.mainloop()


if __name__ == "__main__":
    main()
