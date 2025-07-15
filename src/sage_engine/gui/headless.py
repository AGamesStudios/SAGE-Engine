"""Headless GUI backend used as fallback."""

from __future__ import annotations

from typing import Any, List

from .base import GuiBackend, GuiEvent


class HeadlessBackend(GuiBackend):
    def create_window(self, width: int, height: int, title: str) -> None:
        pass

    def process_events(self) -> List[GuiEvent]:
        return []

    def present(self, texture_handle: Any) -> None:
        pass
