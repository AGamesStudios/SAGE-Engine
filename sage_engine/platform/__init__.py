"""Platform selection and backend helpers."""
from __future__ import annotations

from .common.utils import detect_platform, DummyWindowBackend
from sage.config import load_platform_config


def get_platform() -> str:
    cfg = load_platform_config()
    forced = cfg.get("force")
    if forced:
        return forced
    return detect_platform()


def get_window_backend():
    plat = get_platform()
    if plat == "windows":
        from .windows.window_win32 import WindowBackend
    elif plat == "macos":
        from .macos.window_cocoa import WindowBackend
    elif plat == "linux":
        from .linux.window_x11 import WindowBackend
    else:
        WindowBackend = DummyWindowBackend
    return WindowBackend


__all__ = ["get_platform", "get_window_backend"]
