from __future__ import annotations

from dataclasses import dataclass
import os

from ..base import Widget
from ..i18n import translate


@dataclass
class Label(Widget):
    text: str = ""

    def draw(self) -> None:
        super().draw()
        from ... import gfx
        from ...logger import logger
        font = self.style.font
        if font and not os.path.exists(font):
            logger.warning("Font not found: %s", font)
            font = None
        gfx.draw_text(
            self.x + self.style.padding,
            self.y + self.style.padding,
            translate(self.text),
            font,
            self.style.fg_color,
        )
