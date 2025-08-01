from __future__ import annotations

import json

from dataclasses import dataclass, field

from .. import resource
from ..logger import logger
from ..render import stats as render_stats

from .texture import Texture

@dataclass
class TextureAtlas:
    texture: Texture = field(default_factory=Texture)
    regions: dict[str, tuple[int, int, int, int]] = field(default_factory=dict)

    def load(self, path: str) -> None:
        try:
            self.texture.load(path)
        except Exception as exc:
            logger.warning("[texture] atlas texture missing: %s", path)
            self.regions = {}
            return
        meta_path = f"{path}.meta"
        try:
            meta_data = json.loads(resource.load(meta_path).decode("utf-8"))
            self.regions = {k: tuple(v) for k, v in meta_data.items()}
        except Exception:
            logger.warning("[texture] atlas metadata not found: %s", meta_path)
            self.regions = {}

    def get_region(self, name: str) -> tuple[int, int, int, int] | None:
        if name in self.regions:
            render_stats.stats["atlas_hits"] += 1
            return self.regions[name]
        render_stats.stats["atlas_misses"] += 1
        return None
