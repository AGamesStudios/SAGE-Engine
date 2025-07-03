from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from ..core.objects import register_object
from ..utils.log import logger


@register_object(
    'map',
    [
        ('map_file', 'map'),
        ('tileset', None),
        ('tile_width', None),
        ('tile_height', None),
        ('width', None),
        ('height', None),
        ('data', None),
        ('visible', None),
        ('metadata', 'metadata'),
    ],
)
@dataclass(slots=True)
class TileMap:
    """Tile map composed of a grid of tiles."""

    map_file: str = ''
    tileset: str = ''
    tile_width: int = 32
    tile_height: int = 32
    width: int = 0
    height: int = 0
    data: List[int] = field(default_factory=list)
    visible: bool = True
    metadata: dict = field(default_factory=dict)

    def __post_init__(self) -> None:
        if self.map_file:
            try:
                self.load_map(self.map_file)
            except Exception:
                logger.exception("Failed to load map %s", self.map_file)

    def load_map(self, path: str) -> None:
        from ..formats import load_sagemap

        info = load_sagemap(path)
        self.tileset = info['tileset']
        self.tile_width = info['tile_width']
        self.tile_height = info['tile_height']
        self.width = info['width']
        self.height = info['height']
        self.data = list(info['data'])

