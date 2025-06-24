from dataclasses import dataclass

@dataclass(slots=True)
class Camera:
    """2D camera object that can be placed in a scene."""

    x: float = 0.0
    y: float = 0.0
    width: int = 640
    height: int = 480
    zoom: float = 1.0
    name: str = "Camera"
    type: str = "camera"

    def view_rect(self) -> tuple[float, float, float, float]:
        """Return the visible world rectangle."""
        return (self.x, self.y,
                self.width / self.zoom,
                self.height / self.zoom)
