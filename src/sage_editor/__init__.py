"""Optional editor package for SAGE Engine."""

from __future__ import annotations

from typing import Callable, Any
import sys

from engine.core.engine import _exception_handler
from engine.utils.log import init_logger, logger

from pathlib import Path

from engine.editor_api import EditorInterface
from engine.plugins import PluginManager


class Editor(EditorInterface):
    """Minimal editor implementing :class:`EditorInterface`."""

    def __init__(self) -> None:
        self._menus: list[tuple[str, Callable[..., Any]]] = []
        self._toolbar: list[tuple[str, Callable[..., Any]]] = []
        default_dir = Path(__file__).resolve().parent / "plugins"
        self.plugins = PluginManager("editor", plugin_dir=str(default_dir))

    def add_menu(self, name: str, callback: Callable[..., Any]) -> None:
        self._menus.append((name, callback))

    def add_toolbar_button(self, name: str, callback: Callable[..., Any]) -> None:
        self._toolbar.append((name, callback))

    def load_plugins(self, paths: list[str] | None = None) -> None:
        """Load editor plugins and initialise them with ``self``."""
        self.plugins.load(self, paths)


def main() -> int:
    """Entry point for the default editor implementation."""
    init_logger()
    sys.excepthook = _exception_handler
    editor = Editor()
    editor.load_plugins()
    if not getattr(editor, "window", None):
        logger.error(
            "No editor window was created. "
            "Ensure PyQt6 is installed and plugins are enabled."
        )
        return 1
    logger.info("SAGE Editor started")
    return 0


__all__ = ["Editor", "main"]

