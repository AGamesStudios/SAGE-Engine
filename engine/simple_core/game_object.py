from dataclasses import dataclass, field

@dataclass
class Transform:
    """Simple transform holding position, rotation and scale."""
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    angle: float = 0.0
    scale_x: float = 1.0
    scale_y: float = 1.0

@dataclass
class GameObject:
    """Standalone object used in :class:`Scene`."""
    name: str = "Object"
    role: str | None = None
    transform: Transform = field(default_factory=Transform)

    def update(self, dt: float) -> None:
        """Update the object each frame. Override in subclasses."""
        pass

    def draw(self, renderer) -> None:
        """Draw the object. This base class draws nothing."""
        pass
