from dataclasses import dataclass

__all__ = ["Environment"]

@dataclass(slots=True)
class Environment:
    """Scene environment settings."""

    background: tuple[int, int, int] = (0, 0, 0)

    def set_background(self, color: tuple[int, int, int]) -> None:
        """Update the background colour."""
        self.background = color

