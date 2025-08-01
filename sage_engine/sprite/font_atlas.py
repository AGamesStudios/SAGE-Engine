from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict

from ..gfx import load_font
from ..logger import logger
from ..render import stats as render_stats
from ..texture import TextureAtlas, TextureCache

@dataclass
class FontTextureAtlas:
    path: str
    size: int
    atlas: TextureAtlas = field(init=False)

    def __post_init__(self) -> None:
        self.atlas = TextureCache.load_atlas(self.path)
        self.font = load_font(self.path, self.size)
        if self.font is None:
            logger.warning("Font not found: %s", self.path)

    def draw_text(self, text: str, x: int, y: int) -> None:
        # Placeholder: real rendering would map characters via atlas
        render_stats.stats["sprites_drawn"] += len(text)
