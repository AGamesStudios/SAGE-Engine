"""Basic 2D sprite animation support."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import List


@dataclass(slots=True)
class Frame:
    """Single animation frame."""

    image: str
    duration: float = 0.1


@dataclass(slots=True)
class Animation:
    """Collection of frames that loops by default."""

    frames: List[Frame]
    loop: bool = True
    _index: int = field(init=False, default=0, repr=False)
    _time: float = field(init=False, default=0.0, repr=False)

    @property
    def image(self) -> str:
        """Return the current frame's image path."""
        if not self.frames:
            return ""
        return self.frames[self._index].image

    def reset(self) -> None:
        """Restart the animation from the first frame."""
        self._index = 0
        self._time = 0.0

    def update(self, dt: float) -> str:
        """Advance the animation by ``dt`` seconds and return the new image."""
        if not self.frames:
            return ""
        self._time += dt
        while self._time >= self.frames[self._index].duration:
            self._time -= self.frames[self._index].duration
            self._index += 1
            if self._index >= len(self.frames):
                if self.loop:
                    self._index = 0
                else:
                    self._index = len(self.frames) - 1
                    break
        return self.frames[self._index].image
