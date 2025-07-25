"""Win32 input backend stub."""
from __future__ import annotations


class InputBackend:
    def boot(self) -> None:
        pass

    def poll(self) -> None:
        pass

    def shutdown(self) -> None:
        pass

    def get_events(self) -> list[object]:
        return []
