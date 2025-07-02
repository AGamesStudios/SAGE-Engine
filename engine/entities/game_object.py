from dataclasses import dataclass, field
from ..utils.log import logger
from collections import OrderedDict
from ..core.math2d import (
    angle_to_quat as _angle_to_quat,
    calc_rect as _calc_rect,
    calc_matrix as _calc_matrix,
    normalize_angle,
)
from PIL import Image
from engine.renderers import Shader
from engine.mesh_utils import Mesh
from ..core.objects import register_object
from ..logic import EventSystem, event_from_dict
from .. import units
from ..core.effects import get_effect

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
        ('shape', None),
        ('x', None),
        ('y', None),
        ('z', None),
        ('name', None),
        ('scale_x', None),
        ('scale_y', None),
        ('angle', None),
        ('pivot_x', None),
        ('pivot_y', None),
        ('flip_x', None),
        ('flip_y', None),
        ('smooth', None),
        ('color', None),
        ('alpha', None),
        ('visible', None),
        ('mesh', 'mesh'),
        ('metadata', 'metadata'),
        ('variables', 'variables'),
        ('public_vars', 'public_vars'),
        ('effects', 'effects'),
        ('shader', 'shader'),
        ('shader_uniforms', 'shader_uniforms'),
    ],
)
@dataclass(slots=True)
class GameObject:
    """Sprite-based object used in scenes."""
    image_path: str = ""
    shape: str | None = None
    mesh: "Mesh | None" = None
    x: float = 0
    y: float = 0
    z: float = 0
    name: str | None = None
    scale_x: float = 1.0
    scale_y: float = 1.0
    angle: float = 0.0
    pivot_x: float = 0.5
    pivot_y: float = 0.5
    flip_x: bool = False
    flip_y: bool = False
    smooth: bool = True
    color: tuple[int, int, int, int] | None = None
    alpha: float = 1.0
    metadata: dict = field(default_factory=dict)
    events: list = field(default_factory=list)
    settings: dict = field(default_factory=dict)
    variables: dict = field(default_factory=dict)  # name -> value
    public_vars: set[str] = field(default_factory=set)
    effects: list = field(default_factory=list)
    shader: dict | None = None
    shader_uniforms: dict = field(default_factory=dict)
    visible: bool = True
    event_system: EventSystem | None = field(init=False, default=None)
    rotation: tuple[float, float, float, float] = field(init=False)
    image: Image.Image | None = field(init=False, default=None)
    width: int = field(init=False, default=0)
    height: int = field(init=False, default=0)
    _dirty: bool = field(init=False, default=True)
    _cached_rect: tuple[float, float, float, float] | None = field(init=False, default=None)
    _cached_matrix: list[float] | None = field(init=False, default=None)
    _compiled_shader: "Shader | None" = field(init=False, default=None)

    _DIRTY_FIELDS = {
        'x', 'y', 'z', 'scale_x', 'scale_y', 'angle', 'pivot_x', 'pivot_y',
        'flip_x', 'flip_y'
    }

    def __setattr__(self, name, value):
        if name in GameObject._DIRTY_FIELDS:
            object.__setattr__(self, '_dirty', True)
        if name == 'angle':
            value = normalize_angle(value)
            object.__setattr__(self, 'rotation', _angle_to_quat(value))
        object.__setattr__(self, name, value)

    @property
    def scale(self) -> float:
        return (self.scale_x + self.scale_y) / 2

    @scale.setter
    def scale(self, value: float):
        self.scale_x = self.scale_y = value

    def __post_init__(self):
        self.angle = normalize_angle(self.angle)
        self.rotation = _angle_to_quat(self.angle)
        if self.name is None:
            self.name = "New Object"
        self._load_image()
        if not self.image_path and self.shape is None:
            self.shape = "square"
        if self.shader and isinstance(self.shader, dict):
            vert = self.shader.get("vertex")
            frag = self.shader.get("fragment")
            if vert and frag:
                try:
                    self._compiled_shader = Shader.from_files(vert, frag)
                except Exception:
                    logger.exception("Failed to load shader %s/%s", vert, frag)


    def update(self, dt: float):
        pass

    def get_shader(self) -> "Shader | None":
        """Return the compiled :class:`Shader` for this object."""
        if self._compiled_shader is None and self.shader and isinstance(self.shader, dict):
            vert = self.shader.get("vertex")
            frag = self.shader.get("fragment")
            if vert and frag:
                try:
                    self._compiled_shader = Shader.from_files(vert, frag)
                except Exception:
                    logger.exception("Failed to load shader %s/%s", vert, frag)
        return self._compiled_shader

    def render_position(
        self, camera, apply_effects: bool = True
    ) -> tuple[float, float]:
        """Return the on-screen position with optional effects."""
        x = self.x
        y = self.y
        if apply_effects and camera:
            for eff in getattr(self, "effects", []):
                etype = eff.get("type")
                handler = get_effect(etype)
                if handler:
                    x, y = handler.apply_position(self, camera, eff, (x, y))
        return x, y

    def render_scale(self, camera, apply_effects: bool = True) -> float:
        """Return an additional scale factor from active effects."""
        scale = 1.0
        if apply_effects and camera:
            for eff in getattr(self, "effects", []):
                handler = get_effect(eff.get("type"))
                if handler:
                    scale = handler.apply_scale(self, camera, eff, scale)
        return scale

    def texture_coords(self, camera, apply_effects: bool = True) -> list[float]:
        """Return texture coordinates adjusted by active effects."""
        coords = [0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 0.0, 1.0]
        if not (apply_effects and camera):
            return coords
        for eff in getattr(self, "effects", []):
            handler = get_effect(eff.get("type"))
            if handler:
                coords = handler.apply_uvs(self, camera, eff, coords)
        return coords


    def _load_image(self):
        """Load the object's image with Pillow."""
        if not self.image_path:
            # No sprite image - default dimensions for shape rendering
            self.image = None
            self.width, self.height = 32, 32
            if self.shape is None:
                self.shape = "square"
            self._dirty = True
            self._cached_rect = None
            self._cached_matrix = None
            return
        else:
            from ..core.resources import get_resource_path
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

    def build_event_system(self, objects, variables=None) -> EventSystem:
        """Construct an EventSystem from the object's event data."""
        vars_dict = self.variables if variables is None else variables
        es = EventSystem(variables=vars_dict)
        for evt in getattr(self, "events", []):
            if isinstance(evt, dict):
                es.add_event(event_from_dict(evt, objects, vars_dict))
        self.event_system = es
        return es
