from dataclasses import dataclass

@dataclass(slots=True)
class Camera:
    """Simple orthographic camera."""
    x: float = 0.0
    y: float = 0.0
    width: int = 640
    height: int = 480
