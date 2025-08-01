
"""Convenience entry point for launching the editor in development."""

from sage_engine import core
from sage_editor.gui_main import main as gui_main


def main() -> None:
    """Initialize the engine and start the GUI."""
    core.auto_setup()
    gui_main()


if __name__ == "__main__":  # pragma: no cover - manual utility
    main()
