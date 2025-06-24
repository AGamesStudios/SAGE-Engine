from dataclasses import dataclass, field
import os
import traceback
import math
import pygame

# cache loaded images so repeated sprites don't reload files
_IMAGE_CACHE: dict[str, pygame.Surface] = {}

def clear_image_cache():
    """Remove all cached images."""
    _IMAGE_CACHE.clear()


def _angle_to_quat(angle: float) -> tuple[float, float, float, float]:
    """Return a quaternion representing rotation around Z."""
    rad = math.radians(angle) / 2.0
    return (0.0, 0.0, math.sin(rad), math.cos(rad))


def _quat_to_angle(quat: tuple[float, float, float, float]) -> float:
    """Return the Z rotation in degrees from a quaternion."""
    z, w = quat[2], quat[3]
    return math.degrees(2.0 * math.atan2(z, w))

@dataclass
class GameObject:
    """Sprite-based object used in scenes."""
    image_path: str = ""
    x: float = 0
    y: float = 0
    z: float = 0
    name: str | None = None
    scale_x: float = 1.0
    scale_y: float = 1.0
    angle: float = 0.0
    color: tuple[int, int, int, int] | None = None
    events: list = field(default_factory=list)
    sprite: pygame.Surface | None = field(init=False, default=None)
    rotation: tuple[float, float, float, float] = field(init=False)

    @property
    def scale(self) -> float:
        return (self.scale_x + self.scale_y) / 2

    @scale.setter
    def scale(self, value: float):
        self.scale_x = self.scale_y = value

    def __post_init__(self):
        self.rotation = _angle_to_quat(self.angle)
        if self.name is None:
            self.name = "New Object"

    @property
    def angle(self) -> float:
        return _quat_to_angle(self.rotation)

    @angle.setter
    def angle(self, value: float):
        self.rotation = _angle_to_quat(value)

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
        img = self.sprite
        if self.scale_x != 1.0 or self.scale_y != 1.0:
            w = max(1, int(img.get_width() * self.scale_x))
            h = max(1, int(img.get_height() * self.scale_y))
            img = pygame.transform.scale(img, (w, h))
        if self.angle != 0.0:
            img = pygame.transform.rotate(img, -self.angle)
        surface.blit(img, (self.x, self.y))

    def rect(self) -> pygame.Rect:
        self._ensure_sprite()
        img = self.sprite
        if self.scale_x != 1.0 or self.scale_y != 1.0:
            w = max(1, int(img.get_width() * self.scale_x))
            h = max(1, int(img.get_height() * self.scale_y))
            img = pygame.transform.scale(img, (w, h))
        if self.angle != 0.0:
            img = pygame.transform.rotate(img, -self.angle)
        return img.get_rect(topleft=(self.x, self.y))
