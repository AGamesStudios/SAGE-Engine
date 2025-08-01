from __future__ import annotations

from dataclasses import dataclass

from .. import resource
from ..format import sageimg
from ..logger import logger
from ..render import stats as render_stats

BYTES_PER_PIXEL = 4

@dataclass
class Texture:
    width: int = 0
    height: int = 0
    pixels: bytes | None = None

    def load(self, path: str) -> None:
        if not path.endswith(".sageimg"):
            logger.error("[texture] external image formats are not supported: %s", path)
            return
        data = resource.load(path)
        self.width, self.height, self.pixels = sageimg.decode(data)
        from .cache import TextureCache
        render_stats.stats["texture_memory_kb"] = TextureCache.memory_usage() // 1024

    def get_size(self) -> tuple[int, int]:
        return self.width, self.height

    def get_region(self, x: int, y: int, w: int, h: int) -> bytes:
        if self.pixels is None:
            return b""
        region = bytearray()
        for row in range(y, min(y + h, self.height)):
            start = (row * self.width + x) * BYTES_PER_PIXEL
            end = start + w * BYTES_PER_PIXEL
            region.extend(self.pixels[start:end])
        return bytes(region)

    def unload(self) -> None:
        self.width = 0
        self.height = 0
        self.pixels = None
        from .cache import TextureCache
        render_stats.stats["texture_memory_kb"] = TextureCache.memory_usage() // 1024
