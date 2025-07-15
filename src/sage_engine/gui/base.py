"""Base interfaces for GUI backends."""

from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Any, List


@dataclass
class GuiEvent:
    """Simplified GUI event."""

    type: str
    payload: Any | None = None


class GuiBackend(ABC):
    """GUI backend protocol."""

    @abstractmethod
    def create_window(self, width: int, height: int, title: str) -> None:
        """Create and show the main window."""

    @abstractmethod
    def process_events(self) -> List[GuiEvent]:
        """Process native events and return a list of engine events."""

    @abstractmethod
    def present(self, texture_handle: Any) -> None:
        """Present the rendered frame."""
