from dataclasses import dataclass, field
from .objects import register_object

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
        ('metadata', 'metadata'),
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
    type: str = "camera"
    metadata: dict = field(default_factory=dict)
    events: list = field(default_factory=list)

    def view_rect(self) -> tuple[float, float, float, float]:
        """Return the visible world rectangle."""
        return (self.x, self.y,
                self.width / self.zoom,
                self.height / self.zoom)

    def update(self, dt: float) -> None:
        """Camera objects currently have no behaviour."""
        pass

    def draw(self, surface) -> None:
        """Camera objects are not drawn with sprites."""
        pass
