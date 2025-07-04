"""Interfaces for editor integration."""

from __future__ import annotations

from typing import Protocol, Callable, Any, runtime_checkable


@runtime_checkable
class EditorInterface(Protocol):
    """Minimal interface expected by optional editors."""

    def add_menu(self, name: str, callback: Callable[..., Any]) -> None:
        """Add a menu item."""
        ...

    def add_toolbar_button(self, name: str, callback: Callable[..., Any]) -> None:
        """Add a toolbar button."""
        ...
