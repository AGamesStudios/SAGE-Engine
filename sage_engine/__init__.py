from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Tuple
import json

from panda3d.core import loadPrcFileData
from ursina import Ursina, window


@dataclass
class EngineConfig:
    """Configuration options for the SAGE Engine."""

    window_size: Tuple[int, int] = (1280, 720)
    title: str = "SAGE Engine Application"
    icon_path: Optional[str] = None
    vsync: bool = True
    fullscreen: bool = False


def load_config(path: Path) -> EngineConfig:
    """Load configuration from a JSON file, returning defaults if missing."""
    config = EngineConfig()
    if path and path.is_file():
        data = json.loads(path.read_text())
        for key, value in data.items():
            if hasattr(config, key):
                if key == "window_size":
                    value = tuple(int(v) for v in value)
                setattr(config, key, value)
    return config


class SageEngine:
    """Lightweight wrapper around Ursina to apply configuration."""

    def __init__(self, config: EngineConfig):
        self.config = config
        loadPrcFileData('', f'win-size {int(config.window_size[0])} {int(config.window_size[1])}')
        loadPrcFileData('', f'window-title {config.title}')
        loadPrcFileData('', f'fullscreen {int(config.fullscreen)}')
        loadPrcFileData('', f'vsync {int(config.vsync)}')

    def start(self, setup_fn):
        app = Ursina()
        if self.config.icon_path:
            icon_path = Path(self.config.icon_path)
            if icon_path.exists():
                window.icon = icon_path
        setup_fn()
        app.run()
