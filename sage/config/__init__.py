from __future__ import annotations

from pathlib import Path
import yaml

DEFAULT_CONFIG = {
    "enable_lua": True,
    "enable_flow": True,
    "enable_python": True,
    "watch_scripts": False,
    "allowed_modules": [],
    "allow_lambda": True,
}

DEFAULT_WINDOW_CONFIG = {
    "width": 1280,
    "height": 720,
    "title": "SAGE Engine Alpha 0.3",
    "vsync": True,
    "resizable": True,
    "fullscreen": False,
}

DEFAULT_FRAMESYNC_CONFIG = {
    "enabled": True,
    "target_fps": 60,
    "allow_drift": False,
    "profile": "balanced",
}

DEFAULT_INPUT_CONFIG = {
    "backend": "dummy",
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


def load_framesync_config() -> dict:
    """Load FrameSync configuration from framesync.yaml."""
    path = Path(__file__).with_name("framesync.yaml")
    if path.is_file():
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        cfg = data.get("framesync", {})
        return {**DEFAULT_FRAMESYNC_CONFIG, **cfg}
    return DEFAULT_FRAMESYNC_CONFIG.copy()


def load_input_config() -> dict:
    """Load input configuration from input.yaml."""
    path = Path(__file__).with_name("input.yaml")
    if path.is_file():
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        cfg = data.get("input", {})
        return {**DEFAULT_INPUT_CONFIG, **cfg}
    return DEFAULT_INPUT_CONFIG.copy()

__all__ = [
    "load_config",
    "DEFAULT_CONFIG",
    "load_window_config",
    "DEFAULT_WINDOW_CONFIG",
    "load_framesync_config",
    "DEFAULT_FRAMESYNC_CONFIG",
    "load_input_config",
    "DEFAULT_INPUT_CONFIG",
]
