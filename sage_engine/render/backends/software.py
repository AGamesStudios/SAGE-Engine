"""In-memory software renderer."""

from __future__ import annotations

import logging
from dataclasses import dataclass


@dataclass
class PixelBlock:
    width: int
    height: int
    pixels: list[list[tuple[int, int, int]]]


class SoftwareRenderer:
    def __init__(self, width: int, height: int):
        self.width = width
        self.height = height
        self.logger = logging.getLogger("render")
        self.pixels = self._blank()

    def _blank(self) -> list[list[tuple[int, int, int]]]:
        return [[(0, 0, 0) for _ in range(self.width)] for _ in range(self.height)]

    def begin_frame(self) -> None:
        self.logger.info("Frame started.")
        self.pixels = self._blank()

    def draw_rect(self, x: int, y: int, w: int, h: int, color: tuple[int, int, int]) -> None:
        for j in range(y, min(y + h, self.height)):
            if j < 0:
                continue
            row = self.pixels[j]
            for i in range(x, min(x + w, self.width)):
                if i < 0:
                    continue
                row[i] = color

    def draw_text(self, x: int, y: int, text: str, color: tuple[int, int, int]) -> None:
        # Each character is drawn as a small block; this is only a placeholder.
        for idx, _ch in enumerate(text):
            self.draw_rect(x + idx * 8, y, 6, 8, color)

    def end_frame(self) -> PixelBlock:
        return PixelBlock(self.width, self.height, self.pixels)
