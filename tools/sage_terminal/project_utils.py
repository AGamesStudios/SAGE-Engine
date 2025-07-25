"""Project layout helpers used by SAGE Terminal."""
from __future__ import annotations

from pathlib import Path
from typing import List

LAYOUT = [
    "main.py",
    "config.yaml",
    "project.yaml",
    "README.md",
    "data",
    "data/scenes",
    "data/objects",
    "data/scripts",
    "data/scripts/scene",
    "data/scripts/object",
    "data/textures",
    "data/ui",
    "data/audio",
    "data/particles",
    "data/shaders",
    "data/fonts",
]


def validate_structure(root: str) -> List[str]:
    """Return list of missing files or directories for *root*."""
    base = Path(root)
    missing: List[str] = []
    for item in LAYOUT:
        if not (base / item).exists():
            missing.append(item)
    return missing


def ensure_layout(root: str) -> None:
    """Create project layout inside *root* if missing."""
    base = Path(root)
    for item in LAYOUT:
        path = base / item
        if path.suffix:
            if not path.exists():
                if path.name == "project.yaml":
                    path.write_text(
                        "name: My Game\nauthor: Unknown\ndescription: SAGE project\nversion: 0.1\n",
                        encoding="utf-8",
                    )
                else:
                    path.touch()
        else:
            path.mkdir(parents=True, exist_ok=True)
            keep = path / ".gitkeep"
            if not keep.exists():
                keep.touch()

__all__ = ["LAYOUT", "validate_structure", "ensure_layout"]
