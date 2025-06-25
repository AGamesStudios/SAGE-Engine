from dataclasses import dataclass, field
import os
import traceback
from ..log import logger
import math
from PIL import Image
from .objects import register_object
from ..logic import EventSystem, Event, condition_from_dict, action_from_dict
from .. import units

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
                except Exception:
                    logger.exception('Failed to load image %s', path)
                    img = Image.new('RGBA', (32, 32), self.color or (255, 255, 255, 255))
        self.image = img
        self.width, self.height = img.size

    def rect(self):
        """Return an axis-aligned bounding box in world units."""
        scale = units.UNITS_PER_METER
        px = self.width * self.pivot_x
        py = self.height * self.pivot_y
        sx = self.scale_x
        sy = self.scale_y
        rad = math.radians(self.angle)
        ca = math.cos(rad)
        sa = math.sin(rad)
        corners = [
            (-px, -py),
            (self.width - px, -py),
            (self.width - px, self.height - py),
            (-px, self.height - py),
        ]
        tx = self.x * scale
        ty = self.y * scale
        xs = []
        ys = []
        for cx, cy in corners:
            cx *= sx
            cy *= sy
            rx = cx * ca - cy * sa
            ry = cx * sa + cy * ca
            xs.append(tx + rx)
            ys.append(ty + ry)
        left = min(xs)
        right = max(xs)
        bottom = min(ys)
        top = max(ys)
        return (left, bottom, right - left, top - bottom)

    def transform_matrix(self):
        """Return a 4x4 column-major transform matrix without PyGLM."""
        ang = math.radians(self.angle)
        ca = math.cos(ang)
        sa = math.sin(ang)
        sx = self.scale_x
        sy = self.scale_y
        px = self.width * self.pivot_x
        py = self.height * self.pivot_y
        m00 = ca * sx
        m01 = -sa * sy
        m10 = sa * sx
        m11 = ca * sy
        scale = units.UNITS_PER_METER
        tx = self.x * scale + px - (m00 * px + m01 * py)
        ty = self.y * scale + py - (m10 * px + m11 * py)
        return [
            m00, m10, 0.0, 0.0,
            m01, m11, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            tx,  ty,  0.0, 1.0,
        ]

    def build_event_system(self, objects, variables) -> EventSystem:
        """Construct an EventSystem from the object's event data."""
        es = EventSystem(variables=variables)
        for evt in getattr(self, "events", []):
            if not isinstance(evt, dict):
                continue
            conditions = []
            for cond in evt.get("conditions", []):
                if not isinstance(cond, dict):
                    continue
                cobj = condition_from_dict(cond, objects, variables)
                if cobj is not None:
                    conditions.append(cobj)
                else:
                    logger.warning('Skipped invalid condition %s', cond)
            actions = []
            for act in evt.get("actions", []):
                if not isinstance(act, dict):
                    continue
                aobj = action_from_dict(act, objects)
                if aobj is not None:
                    actions.append(aobj)
                else:
                    logger.warning('Skipped invalid action %s', act)
            es.add_event(Event(conditions, actions, evt.get("once", False)))
        self.event_system = es
        return es
