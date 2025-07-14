
from dataclasses import dataclass, field
from .objects import register_object
from ..logic import EventSystem, event_from_dict
from ..utils.log import logger
from ..utils import units
from engine.renderers import Shader

@register_object(
    'camera',
    [
        ('x', None),
        ('y', None),
        ('z', None),
        ('width', None),
        ('height', None),
        ('zoom', None),
        ('name', None),
        ('active', 'active'),
        ('metadata', 'metadata'),
        ('variables', 'variables'),
        ('public_vars', 'public_vars'),
        ('shader', 'shader'),
        ('shader_uniforms', 'shader_uniforms'),
    ],
)
@dataclass(slots=True)
class Camera:
    """2D camera object that can be placed in a scene."""

    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    width: int = 640
    height: int = 480
    zoom: float = 1.0
    name: str = "Camera"
    active: bool = False
    type: str = "camera"
    shader: dict | None = None
    shader_uniforms: dict = field(default_factory=dict)
    metadata: dict = field(default_factory=dict)
    variables: dict = field(default_factory=dict)
    public_vars: set[str] = field(default_factory=set)
    events: list = field(default_factory=list)
    event_system: EventSystem | None = field(init=False, default=None)
    _compiled_shader: "Shader | None" = field(init=False, default=None)

    def __post_init__(self):
        if isinstance(self.shader, dict):
            vert = self.shader.get("vertex")
            frag = self.shader.get("fragment")
            if vert and frag:
                try:
                    self._compiled_shader = Shader.from_files(vert, frag)
                except Exception:
                    logger.exception("Failed to load shader %s/%s", vert, frag)

    def view_rect(self) -> tuple[float, float, float, float]:
        """Return the visible world rectangle with the camera centered."""
        scale = units.UNITS_PER_METER
        w = (self.width / self.zoom) * scale
        h = (self.height / self.zoom) * scale
        left = self.x * scale - w / 2
        bottom = self.y * scale - h / 2
        return (left, bottom, w, h)

    def update(self, dt: float) -> None:
        """Camera objects currently have no behaviour."""
        pass

    def draw(self, surface) -> None:
        """Camera objects are not drawn with sprites."""
        pass

    def get_shader(self) -> "Shader | None":
        """Return the compiled shader for this camera."""
        if self._compiled_shader is None and isinstance(self.shader, dict):
            vert = self.shader.get("vertex")
            frag = self.shader.get("fragment")
            if vert and frag:
                try:
                    self._compiled_shader = Shader.from_files(vert, frag)
                except Exception:
                    logger.exception("Failed to load shader %s/%s", vert, frag)
        return self._compiled_shader

    def build_event_system(self, objects, variables=None) -> EventSystem:
        """Build and store an EventSystem from the attached events."""
        vars_dict = self.variables if variables is None else variables
        es = EventSystem(variables=vars_dict)
        for evt in getattr(self, "events", []):
            if isinstance(evt, dict):
                es.add_event(event_from_dict(evt, objects, vars_dict))
        self.event_system = es
        return es
