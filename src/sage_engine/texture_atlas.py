"""Simple texture atlas for sprite sheets."""

from PIL import Image  # type: ignore[import-not-found]

from .core.resources import get_resource_path


class TextureAtlas:
    """Load an image and expose tiles by index."""

    def __init__(self, image: str, tile_width: int, tile_height: int) -> None:
        path = get_resource_path(image)
        self.image = Image.open(path).convert("RGBA")
        self.tile_width = tile_width
        self.tile_height = tile_height
        self.cols = self.image.width // tile_width
        self.rows = self.image.height // tile_height

    def get_tile(self, index: int) -> Image.Image:
        col = index % self.cols
        row = index // self.cols
        box = (
            col * self.tile_width,
            row * self.tile_height,
            col * self.tile_width + self.tile_width,
            row * self.tile_height + self.tile_height,
        )
        return self.image.crop(box)

__all__ = ["TextureAtlas"]
