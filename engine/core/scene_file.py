from __future__ import annotations

from dataclasses import dataclass, field
import json
import os

from .scenes import Scene
from ..utils import load_json
from ..utils.log import logger

__all__ = ["SceneFile", "load_scene_file", "save_scene_file"]


@dataclass(slots=True)
class SceneFile:
    """Wrapper storing a :class:`Scene` with convenience load/save methods."""

    scene: Scene = field(default_factory=Scene)

    @classmethod
    def load(cls, path: str) -> "SceneFile":
        p = str(path)
        if not p.endswith(".sagescene"):
            logger.warning("Loading scene with unusual extension: %s", path)
        data = load_json(p)
        scene = Scene.from_dict(data)
        return cls(scene)

    def save(self, path: str) -> None:
        p = str(path)
        if not p.endswith(".sagescene"):
            logger.warning("Saving scene with unusual extension: %s", path)
        os.makedirs(os.path.dirname(p) or ".", exist_ok=True)
        with open(p, "w", encoding="utf-8") as f:
            json.dump(self.scene.to_dict(), f, indent=2)


def load_scene_file(path: str) -> Scene:
    """Return a :class:`Scene` loaded from ``path``."""
    return SceneFile.load(path).scene


def save_scene_file(scene: Scene, path: str) -> None:
    """Save ``scene`` to ``path`` in ``.sagescene`` format."""
    SceneFile(scene).save(path)
