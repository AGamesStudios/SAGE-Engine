"""Basic 2D sprite animation support."""


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
    speed: float = 1.0
    playing: bool = True
    reverse: bool = False
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
        if self.reverse and self.frames:
            self._index = len(self.frames) - 1
        else:
            self._index = 0
        self._time = 0.0

    def play(self) -> None:
        """Resume playback."""
        self.playing = True

    def pause(self) -> None:
        """Pause playback without resetting."""
        self.playing = False

    def stop(self) -> None:
        """Stop and reset the animation."""
        self.pause()
        self.reset()

    def update(self, dt: float) -> str:
        """Advance the animation by ``dt`` seconds and return the new image."""
        if not self.frames:
            return ""
        if not self.playing:
            return self.frames[self._index].image
        self._time += dt * max(self.speed, 0.0)
        while self._time >= self.frames[self._index].duration:
            self._time -= self.frames[self._index].duration
            step = -1 if self.reverse else 1
            self._index += step
            if self._index >= len(self.frames) or self._index < 0:
                if self.loop:
                    self._index %= len(self.frames)
                else:
                    self._index = 0 if self.reverse else len(self.frames) - 1
                    self.playing = False
                    self._time = 0.0
                    break
        return self.frames[self._index].image
