from __future__ import annotations

from pathlib import Path
import yaml

DEFAULT_CONFIG = {
    "enable_lua": True,
    "enable_flow": True,
    "watch_scripts": False,
}


def load_config() -> dict:
    """Load script configuration from scripts.yaml."""
    path = Path(__file__).with_name("scripts.yaml")
    if path.is_file():
        data = yaml.safe_load(path.read_text(encoding="utf-8")) or {}
        return {**DEFAULT_CONFIG, **data}
    return DEFAULT_CONFIG.copy()

__all__ = ["load_config", "DEFAULT_CONFIG"]
