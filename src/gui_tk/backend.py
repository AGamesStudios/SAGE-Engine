"""Tkinter implementation of GuiBackend."""

from __future__ import annotations

from typing import Any, List
import tkinter as tk

from sage_engine.gui.base import GuiBackend, GuiEvent


class TkBackend(GuiBackend):
    def __init__(self) -> None:
        self.root: tk.Tk | None = None
        self._created = False

    def create_window(self, width: int, height: int, title: str) -> None:
        if self.root is None:
            self.root = tk.Tk()
            self.root.withdraw()
        if not self._created:
            self.root.deiconify()
            self.root.title(title)
            self.root.geometry(f"{width}x{height}")
            self.root.protocol("WM_DELETE_WINDOW", self.root.destroy)
            self._created = True

    def process_events(self) -> List[GuiEvent]:
        if self.root is not None:
            self.root.update()
        return []

    def present(self, texture_handle: Any) -> None:
        pass
