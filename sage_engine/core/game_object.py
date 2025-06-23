from dataclasses import dataclass, field
import os
import traceback
import pygame

# cache loaded images so repeated sprites don't reload files
_IMAGE_CACHE: dict[str, pygame.Surface] = {}

def clear_image_cache():
    """Remove all cached images."""
    _IMAGE_CACHE.clear()

@dataclass
class GameObject:
    """Sprite-based object used in scenes."""
    image_path: str
    x: float = 0
    y: float = 0
    name: str | None = None
    events: list = field(default_factory=list)
    sprite: pygame.Surface | None = field(init=False, default=None)

    def __post_init__(self):
        if self.name is None:
            self.name = os.path.basename(self.image_path)

    def update(self, dt: float):
        pass

    def _ensure_sprite(self):
        if self.sprite is None:
            sprite = _IMAGE_CACHE.get(self.image_path)
            if sprite is None:
                try:
                    sprite = pygame.image.load(self.image_path).convert_alpha()
                    _IMAGE_CACHE[self.image_path] = sprite
                except Exception as exc:
                    print(f'Failed to load image {self.image_path}: {exc}')
                    traceback.print_exc()
                    sprite = pygame.Surface((10, 10), pygame.SRCALPHA)
            self.sprite = sprite

    def draw(self, surface: pygame.Surface):
        self._ensure_sprite()
        surface.blit(self.sprite, (self.x, self.y))

    def rect(self) -> pygame.Rect:
        self._ensure_sprite()
        return self.sprite.get_rect(topleft=(self.x, self.y))
