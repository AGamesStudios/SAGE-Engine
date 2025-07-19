from __future__ import annotations

from pathlib import Path
import yaml

DEFAULT_CONFIG = {
    "enable_lua": True,
    "enable_flow": True,
    "watch_scripts": False,
}

DEFAULT_WINDOW_CONFIG = {
    "width": 1280,
    "height": 720,
    "title": "SAGE Engine Alpha 0.3",
    "vsync": True,
    "resizable": True,
    "fullscreen": False,
}


def load_config() -> dict:
    """Load script configuration from scripts.yaml."""
    path = Path(__file__).with_name("scripts.yaml")
    if path.is_file():
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        return {**DEFAULT_CONFIG, **data}
    return DEFAULT_CONFIG.copy()


def load_window_config() -> dict:
    """Load window configuration from window.yaml."""
    path = Path(__file__).with_name("window.yaml")
    if path.is_file():
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        cfg = data.get("window", {})
        return {**DEFAULT_WINDOW_CONFIG, **cfg}
    return DEFAULT_WINDOW_CONFIG.copy()

__all__ = [
    "load_config",
    "DEFAULT_CONFIG",
    "load_window_config",
    "DEFAULT_WINDOW_CONFIG",
]
