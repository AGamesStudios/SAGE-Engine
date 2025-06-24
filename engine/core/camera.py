from dataclasses import dataclass

@dataclass(slots=True)
class Camera:
    """2D camera with optional zoom."""

    x: float = 0.0
    y: float = 0.0
    width: int = 640
    height: int = 480
    zoom: float = 1.0

    def view_rect(self) -> tuple[float, float, float, float]:
        """Return the visible world rectangle."""
        return (self.x, self.y,
                self.width / self.zoom,
                self.height / self.zoom)
