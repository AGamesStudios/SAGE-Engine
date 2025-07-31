"""Minimal GUI context for SAGE GUI.

This module manages widgets and dispatches input events.
It is designed to be independent from Tkinter and relies on
SAGE Engine's render and input systems.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Optional

from .event import Event, EventType


@dataclass
class GUIContext:
    """Central context managing widgets and event flow."""

    widgets: List["BaseWidget"] = field(default_factory=list)
    focused: Optional["BaseWidget"] = None

    def add_widget(self, widget: "BaseWidget") -> None:
        self.widgets.append(widget)

    def update(self, dt: float) -> None:
        for widget in self.widgets:
            widget.update(self, dt)

    def draw(self) -> None:
        for widget in self.widgets:
            widget.draw(self)

    def dispatch_event(self, event: Event) -> None:
        for widget in reversed(self.widgets):
            if widget.handle_event(self, event):
                break


# avoid circular import
from ..widgets.base_widget import BaseWidget  # noqa: E402
