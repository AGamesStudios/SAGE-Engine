from __future__ import annotations

"""Load worlds from configuration files."""

from pathlib import Path
from .context import WorldConfig
from .parser import parse_world_file


def load_world(path: str | Path) -> WorldConfig:
    """Parse and return a :class:`WorldConfig`."""
    return parse_world_file(path)
