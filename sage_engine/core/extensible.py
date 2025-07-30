from __future__ import annotations

from typing import Any, Protocol

class IExtensible(Protocol):
    """Extension interface for optional modules."""

    def on_attach(self, engine: Any) -> None:
        """Called when the extension is registered."""
        ...

    def on_shutdown(self) -> None:
        """Called when the extension is removed or engine shuts down."""
        ...

