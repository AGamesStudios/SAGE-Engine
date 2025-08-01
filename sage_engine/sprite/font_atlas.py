from __future__ import annotations

from dataclasses import dataclass, field

import os

from ..gfx import load_font
from ..logger import logger
from ..render import stats as render_stats
from ..texture import TextureAtlas, TextureCache

@dataclass
class FontTextureAtlas:
    path: str
    size: int
    atlas: TextureAtlas = field(init=False)
    font: object | None = None

    def __post_init__(self) -> None:
        atlas_path = self.path
        font_path = self.path
        if self.path.endswith(".sageimg"):
            font_path = self.path.replace(".sageimg", ".ttf")
        else:
            atlas_path = self.path.replace(".ttf", ".sageimg")
        if os.path.exists(atlas_path):
            self.atlas = TextureCache.load_atlas(atlas_path)
        else:
            self.atlas = TextureAtlas()
        self.font = load_font(font_path, self.size)
        if self.font is None:
            logger.warning("Font not found: %s", font_path)

    def draw_text(self, text: str, x: int, y: int) -> None:
        render_stats.stats["text_glyphs_rendered"] += len(text)
