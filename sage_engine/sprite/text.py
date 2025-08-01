from __future__ import annotations

from __future__ import annotations

from pathlib import Path

from .font_atlas import FontTextureAtlas
from .. import gfx
from ..logger import logger
from ..render import stats as render_stats

_DEFAULT_FONT_PATH = str(Path(__file__).resolve().parents[1] / "resources" / "fonts" / "default.ttf")
_default_font: FontTextureAtlas | None = None


def load_font(path: str, size: int) -> FontTextureAtlas:
    """Load a font atlas from *path*."""
    atlas = FontTextureAtlas(path, size)
    return atlas


def _ensure_default(size: int) -> FontTextureAtlas:
    global _default_font
    if _default_font is None or _default_font.size != size:
        _default_font = FontTextureAtlas(_DEFAULT_FONT_PATH, size)
    return _default_font


def draw_text(text: str, x: int, y: int, font: FontTextureAtlas | None = None) -> None:
    """Draw *text* at coordinates."""
    if font is None:
        font = _ensure_default(14)
    if font.font is None:
        logger.warning("Font not found: %s", font.path)
        font = _ensure_default(14)
        if font.font is None:
            return
    gfx.draw_text(x, y, text, font.font)
    render_stats.stats["text_glyphs_rendered"] += len(text)

