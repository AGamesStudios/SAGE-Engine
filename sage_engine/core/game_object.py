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
    image_path: str = ""
    x: float = 0
    y: float = 0
    name: str | None = None
    scale: float = 1.0
    angle: float = 0.0
    color: tuple[int, int, int, int] | None = None
    events: list = field(default_factory=list)
    sprite: pygame.Surface | None = field(init=False, default=None)

    def __post_init__(self):
        if self.name is None:
            self.name = os.path.basename(self.image_path)

    def update(self, dt: float):
        pass

    def _ensure_sprite(self):
        if self.sprite is None:
            if self.image_path:
                sprite = _IMAGE_CACHE.get(self.image_path)
                if sprite is None:
                    try:
                        sprite = pygame.image.load(self.image_path).convert_alpha()
                        _IMAGE_CACHE[self.image_path] = sprite
                    except Exception as exc:
                        print(f'Failed to load image {self.image_path}: {exc}')
                        traceback.print_exc()
                        sprite = pygame.Surface((32, 32), pygame.SRCALPHA)
                        sprite.fill(self.color or (255, 255, 255, 255))
                self.sprite = sprite
            else:
                sprite = pygame.Surface((32, 32), pygame.SRCALPHA)
                sprite.fill(self.color or (255, 255, 255, 255))
                self.sprite = sprite

    def draw(self, surface: pygame.Surface):
        self._ensure_sprite()
        img = pygame.transform.rotozoom(self.sprite, -self.angle, self.scale)
        surface.blit(img, (self.x, self.y))

    def rect(self) -> pygame.Rect:
        self._ensure_sprite()
        img = pygame.transform.rotozoom(self.sprite, -self.angle, self.scale)
        return img.get_rect(topleft=(self.x, self.y))
