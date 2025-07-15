
from dataclasses import dataclass, field
from typing import List

try:  # optional dependency for caching in SDL renderer
    import sdl2
except Exception:  # pragma: no cover - optional
    sdl2 = None  # type: ignore

from ..core.objects import register_object
from ..utils.log import logger


@register_object(
    'map',
    [
        ('map_file', 'map_file'),
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
    name: str = "TileMap"
    tileset: str = ''
    tile_width: int = 32
    tile_height: int = 32
    width: int = 0
    height: int = 0
    data: List[int] = field(default_factory=list)
    visible: bool = True
    metadata: dict = field(default_factory=dict)
    _texture: object | None = field(init=False, default=None, repr=False)

    def __post_init__(self) -> None:
        if self.map_file:
            try:
                self.load_map(self.map_file)
            except Exception:
                logger.exception("Failed to load map %s", self.map_file)

    def load_map(self, path: str) -> None:
        from ..formats import load_sagemap
        from ..core.resources import get_resource_path

        info = load_sagemap(get_resource_path(path))
        self.tileset = info['tileset']
        self.tile_width = info['tile_width']
        self.tile_height = info['tile_height']
        self.width = info['width']
        self.height = info['height']
        self.data = list(info['data'])
        self._texture = None

    # ------------------------------------------------------------------
    def clear_cache(self) -> None:
        """Discard any cached renderer data."""
        if self._texture and sdl2 is not None:
            try:
                sdl2.SDL_DestroyTexture(self._texture)
            except Exception:  # pragma: no cover - SDL may be stubbed
                logger.exception("SDL_DestroyTexture failed")
        self._texture = None

