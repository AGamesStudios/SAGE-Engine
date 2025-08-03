from __future__ import annotations

from dataclasses import dataclass
import os

from sage_engine.gui.base import Widget
from sage_engine.gui.i18n import translate
import sage_engine.gfx as gfx
from sage_engine.logger import logger


@dataclass
class Label(Widget):
    text: str = ""

    def draw(self) -> None:
        super().draw()
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
