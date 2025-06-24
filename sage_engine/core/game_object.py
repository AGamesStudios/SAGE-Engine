from dataclasses import dataclass, field
import os
import traceback
import math
from PIL import Image

# cache loaded images so repeated sprites don't reload files
_IMAGE_CACHE: dict[str, Image.Image] = {}

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
    settings: dict = field(default_factory=dict)
    rotation: tuple[float, float, float, float] = field(init=False)
    image: Image.Image | None = field(init=False, default=None)
    width: int = field(init=False, default=0)
    height: int = field(init=False, default=0)

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
        self._load_image()
    @property
    def angle(self) -> float:
        return _quat_to_angle(self.rotation)

    @angle.setter
    def angle(self, value: float):
        self.rotation = _angle_to_quat(value)

    def update(self, dt: float):
        pass


    def _load_image(self):
        """Load the object's image with Pillow."""
        if not self.image_path:
            img = Image.new('RGBA', (32, 32), self.color or (255, 255, 255, 255))
        else:
            from .resources import get_resource_path
            path = get_resource_path(self.image_path)
            img = _IMAGE_CACHE.get(path)
            if img is None:
                try:
                    img = Image.open(path).convert('RGBA')
                    _IMAGE_CACHE[path] = img
                except Exception as exc:
                    print(f'Failed to load image {path}: {exc}')
                    traceback.print_exc()
                    img = Image.new('RGBA', (32, 32), self.color or (255, 255, 255, 255))
        self.image = img
        self.width, self.height = img.size

    def rect(self):
        return (self.x, self.y, self.width * self.scale_x, self.height * self.scale_y)

    def transform_matrix(self):
        """Return a 4x4 column-major matrix for OpenGL."""
        rad = math.radians(self.angle)
        cos_a = math.cos(rad)
        sin_a = math.sin(rad)
        sx = self.scale_x
        sy = self.scale_y
        px = self.width / 2
        py = self.height / 2
        tx = self.x + px * sx * cos_a - py * sy * sin_a
        ty = self.y + px * sx * sin_a + py * sy * cos_a
        return [
            cos_a * sx, -sin_a * sy, 0, 0,
            sin_a * sx, cos_a * sy, 0, 0,
            0, 0, 1, 0,
            tx, ty, 0, 1,
        ]
