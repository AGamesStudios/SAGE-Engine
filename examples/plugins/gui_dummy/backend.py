"""Example GUI backend printing frames."""
from __future__ import annotations

from typing import Any, List

from sage_engine.gui.base import GuiBackend, GuiEvent


class DummyBackend(GuiBackend):
    def create_window(self, width: int, height: int, title: str) -> None:
        print(f"dummy window {title} {width}x{height}")

    def process_events(self) -> List[GuiEvent]:
        return []

    def present(self, texture_handle: Any) -> None:
        print("present frame")
