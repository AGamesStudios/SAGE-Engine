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
    from ursina import Ursina, window, color
    from ursina.lights import DirectionalLight, AmbientLight
except Exception:  # pragma: no cover - runtime check
    Ursina = None
    window = None
    color = None
    DirectionalLight = None
    AmbientLight = None

VERSION = "0.0.20a"
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
    show_fps: bool = False
    max_fps: Optional[int] = None
    ambient_color: Tuple[float, float, float] = (1.0, 1.0, 1.0)
    enable_shadows: bool = False
    shadow_map_size: int = 1024
    anti_aliasing: bool = True
    texture_filtering: str = "linear"
    render_path: str = "forward"


def validate_config(config: EngineConfig) -> None:
    """Validate configuration values raising ``ValueError`` if invalid."""
    if len(config.window_size) != 2 or not all(isinstance(v, int) and v > 0 for v in config.window_size):
        raise ValueError("window_size must contain two positive integers")
    if config.max_fps is not None and (not isinstance(config.max_fps, int) or config.max_fps <= 0):
        raise ValueError("max_fps must be a positive integer if provided")
    if any(not (0.0 <= c <= 1.0) for c in config.ambient_color):
        raise ValueError("ambient_color values must be between 0 and 1")
    if config.shadow_map_size <= 0:
        raise ValueError("shadow_map_size must be positive")
    if config.texture_filtering not in {"linear", "nearest"}:
        raise ValueError("texture_filtering must be 'linear' or 'nearest'")
    if config.render_path not in {"forward", "deferred"}:
        raise ValueError("render_path must be 'forward' or 'deferred'")


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
                if key == "max_fps" and value is not None:
                    value = int(value)
                if key == "ambient_color":
                    value = tuple(float(v) for v in value)
                if key in {"shadow_map_size"}:
                    value = int(value)
                setattr(config, key, value)
    return config


class SageEngine:
    """Lightweight wrapper around Ursina to apply configuration."""

    def __init__(self, config: EngineConfig):
        validate_config(config)
        self.config = config
        self._apply_prc_settings()
        self._directional_light = None

    def _setup_graphics(self):
        """Create lights and other visual settings based on configuration."""
        if AmbientLight is None:
            return
        if self.config.ambient_color:
            ambient = AmbientLight()
            ambient.color = color.rgb(
                int(self.config.ambient_color[0] * 255),
                int(self.config.ambient_color[1] * 255),
                int(self.config.ambient_color[2] * 255),
            )
        if self.config.enable_shadows:
            self._directional_light = DirectionalLight(shadows=True)
            self._directional_light.shadow_resolution = self.config.shadow_map_size

    def _apply_prc_settings(self) -> None:
        loadPrcFileData('', f'win-size {int(self.config.window_size[0])} {int(self.config.window_size[1])}')
        loadPrcFileData('', f'window-title {self.config.title}')
        loadPrcFileData('', f'fullscreen {int(self.config.fullscreen)}')
        loadPrcFileData('', f'vsync {int(self.config.vsync)}')
        if self.config.anti_aliasing:
            loadPrcFileData('', 'framebuffer-multisample 1')
            loadPrcFileData('', 'multisamples 4')
        if self.config.texture_filtering == 'nearest':
            loadPrcFileData('', 'textures-power-2 none')
        if self.config.render_path == 'deferred':
            loadPrcFileData('', 'render-path Deferred')
        if self.config.show_fps:
            loadPrcFileData('', 'show-frame-rate-meter 1')
        if self.config.max_fps:
            loadPrcFileData('', 'clock-mode limited')
            loadPrcFileData('', f'clock-frame-rate {int(self.config.max_fps)}')

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
        if self.config.show_fps:
            window.fps_counter.enabled = True
        self._setup_graphics()
        setup_fn()
        app.run()
