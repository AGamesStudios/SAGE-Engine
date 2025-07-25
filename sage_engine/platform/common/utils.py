"""Shared helpers for platform backends."""
from __future__ import annotations

import sys


class DummyWindowBackend:
    """Fallback window backend used in tests and headless mode."""

    def __init__(self, width: int, height: int, title: str, **kw) -> None:
        self.width = width
        self.height = height
        self.title = title

    def boot(self) -> None:
        pass

    def poll(self) -> None:
        pass

    def shutdown(self) -> None:
        pass


def detect_platform() -> str:
    """Return simplified platform name."""
    sys_plat = sys.platform
    if sys_plat.startswith("win"):
        return "windows"
    if sys_plat == "darwin":
        return "macos"
    if sys_plat.startswith("linux"):
        return "linux"
    return "unknown"
