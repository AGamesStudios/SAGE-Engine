"""Core module for the SAGE Engine prototype."""

from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Tuple
import json
import logging
import platform
import sys

from panda3d.core import loadPrcFileData

try:  # Optional import so unit tests don't require Ursina
    from ursina import Ursina, window
except Exception:  # pragma: no cover - runtime check
    Ursina = None
    window = None

VERSION = "0.0.1a"
DEFAULT_CONFIG_PATH = Path(__file__).resolve().parent.parent / "sage_config.json"


@dataclass
class EngineConfig:
    """Configuration options for the SAGE Engine."""

    window_size: Tuple[int, int] = (1280, 720)
    title: str = "SAGE Engine Application"
    icon_path: Optional[str] = None
    asset_path: Optional[str] = None
    vsync: bool = True
    fullscreen: bool = False
    debug: bool = False


def validate_config(config: EngineConfig) -> None:
    """Validate configuration values raising ``ValueError`` if invalid."""
    if len(config.window_size) != 2 or not all(isinstance(v, int) and v > 0 for v in config.window_size):
        raise ValueError("window_size must contain two positive integers")


def system_info() -> str:
    """Return a short string describing the host system."""
    return f"{platform.system()} {platform.release()} | Python {sys.version.split()[0]}"


def load_config(path: Optional[Path] = None) -> EngineConfig:
    """Load configuration from a JSON file, returning defaults if missing."""
    path = path or DEFAULT_CONFIG_PATH
    config = EngineConfig()
    if path and path.is_file():
        try:
            data = json.loads(path.read_text())
        except json.JSONDecodeError as exc:  # malformed JSON
            logging.error("Failed to parse %s: %s", path, exc)
            return config
        for key, value in data.items():
            if hasattr(config, key):
                if key == "window_size":
                    value = tuple(int(v) for v in value)
                setattr(config, key, value)
    return config


class SageEngine:
    """Lightweight wrapper around Ursina to apply configuration."""

    def __init__(self, config: EngineConfig):
        validate_config(config)
        self.config = config
        self._apply_prc_settings()

    def _apply_prc_settings(self) -> None:
        loadPrcFileData('', f'win-size {int(self.config.window_size[0])} {int(self.config.window_size[1])}')
        loadPrcFileData('', f'window-title {self.config.title}')
        loadPrcFileData('', f'fullscreen {int(self.config.fullscreen)}')
        loadPrcFileData('', f'vsync {int(self.config.vsync)}')

    def start(self, setup_fn):
        if Ursina is None:
            raise ImportError("Ursina package is required to run the engine")

        app = Ursina()
        if self.config.icon_path:
            icon_path = Path(self.config.icon_path)
            if icon_path.exists():
                window.icon = icon_path
        if self.config.asset_path:
            asset_path = Path(self.config.asset_path)
            if asset_path.is_dir():
                app.asset_folder = asset_path
        if self.config.debug:
            logging.info("System info: %s", system_info())
            logging.info("Loaded config: %s", self.config)
        setup_fn()
        app.run()
