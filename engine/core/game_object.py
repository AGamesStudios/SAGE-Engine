from dataclasses import dataclass, field
import os
import traceback
from ..log import logger
import math
from collections import OrderedDict
from .fastmath import (
    angle_to_quat as _angle_to_quat,
    quat_to_angle as _quat_to_angle,
    calc_rect as _calc_rect,
    calc_matrix as _calc_matrix,
)
from PIL import Image
from .objects import register_object
from ..logic import EventSystem, event_from_dict
from .. import units

# LRU cache so repeated sprites don't reload files on low spec machines
_IMAGE_CACHE: "OrderedDict[str, Image.Image]" = OrderedDict()
_MAX_CACHE = 32

def clear_image_cache():
    """Remove all cached images."""
    _IMAGE_CACHE.clear()



@register_object(
    'sprite',
    [
        ('image_path', 'image'),
        ('x', None),
        ('y', None),
        ('z', None),
        ('name', None),
        ('scale_x', None),
        ('scale_y', None),
        ('angle', None),
        ('pivot_x', None),
        ('pivot_y', None),
        ('color', None),
        ('metadata', 'metadata'),
    ],
)
@dataclass(slots=True)
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
    pivot_x: float = 0.5
    pivot_y: float = 0.5
    color: tuple[int, int, int, int] | None = None
    metadata: dict = field(default_factory=dict)
    events: list = field(default_factory=list)
    settings: dict = field(default_factory=dict)
    event_system: EventSystem | None = field(init=False, default=None)
    rotation: tuple[float, float, float, float] = field(init=False)
    image: Image.Image | None = field(init=False, default=None)
    width: int = field(init=False, default=0)
    height: int = field(init=False, default=0)
    _dirty: bool = field(init=False, default=True)
    _cached_rect: tuple[float, float, float, float] | None = field(init=False, default=None)
    _cached_matrix: list[float] | None = field(init=False, default=None)

    _DIRTY_FIELDS = {
        'x', 'y', 'z', 'scale_x', 'scale_y', 'angle', 'pivot_x', 'pivot_y'
    }

    def __setattr__(self, name, value):
        if name in GameObject._DIRTY_FIELDS:
            object.__setattr__(self, '_dirty', True)
        if name == 'angle':
            object.__setattr__(self, 'rotation', _angle_to_quat(value))
        object.__setattr__(self, name, value)

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
                    while len(_IMAGE_CACHE) > _MAX_CACHE:
                        _IMAGE_CACHE.popitem(last=False)
                except Exception:
                    logger.exception('Failed to load image %s', path)
                    img = Image.new('RGBA', (32, 32), self.color or (255, 255, 255, 255))
        self.image = img
        self.width, self.height = img.size
        self._dirty = True
        self._cached_rect = None
        self._cached_matrix = None

    def rect(self):
        """Return an axis-aligned bounding box in world units."""
        if not self._dirty and self._cached_rect is not None:
            return self._cached_rect
        left, bottom, width, height = _calc_rect(
            self.x, self.y, self.width, self.height,
            self.pivot_x, self.pivot_y,
            self.scale_x, self.scale_y,
            self.angle, units.UNITS_PER_METER, units.Y_UP,
        )
        self._cached_rect = (left, bottom, width, height)
        self._dirty = False
        return self._cached_rect

    def transform_matrix(self):
        """Return a 4x4 column-major transform matrix without PyGLM."""
        if not self._dirty and self._cached_matrix is not None:
            return self._cached_matrix
        self._cached_matrix = list(_calc_matrix(
            self.x, self.y, self.width, self.height,
            self.pivot_x, self.pivot_y,
            self.scale_x, self.scale_y,
            self.angle, units.UNITS_PER_METER, units.Y_UP,
        ))
        self._dirty = False
        return self._cached_matrix

    def build_event_system(self, objects, variables) -> EventSystem:
        """Construct an EventSystem from the object's event data."""
        es = EventSystem(variables=variables)
        for evt in getattr(self, "events", []):
            if isinstance(evt, dict):
                es.add_event(event_from_dict(evt, objects, variables))
        self.event_system = es
        return es
