from __future__ import annotations

from typing import Protocol, List


class BackendBase(Protocol):
    """Interface for input backends."""

    def boot(self) -> None:
        """Initialise the backend."""

    def poll(self) -> None:
        """Poll system events and update state."""

    def shutdown(self) -> None:
        """Shutdown the backend."""

    def get_events(self) -> List[object]:
        """Return collected events since last poll."""
