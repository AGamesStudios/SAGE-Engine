from __future__ import annotations

from typing import Callable, Dict, List

from .sageanim import decode
from .types import AnimationData, FrameData
from ..resource import load as load_resource
from ..sprite import draw, sprite as sprite_mod


class AnimationPlayer:
    """Simple frame-by-frame animation player."""

    def __init__(self, path: str, target_sprite: sprite_mod.Sprite):
        data = load_resource(path)
        frames, durations, loop, events = decode(data)
        self.animation = AnimationData(
            [FrameData(i, d, e or None) for i, d, e in zip(frames, durations, events)],
            loop,
        )
        self.sprite = target_sprite
        self.frame_index = 0
        self.elapsed = 0
        self.playing = False
        self._callbacks: Dict[str, List[Callable[[], None]]] = {}

    def play(self) -> None:
        self.playing = True
        self.frame_index = 0
        self.elapsed = 0

    def pause(self) -> None:
        self.playing = False

    def stop(self) -> None:
        self.playing = False
        self.frame_index = 0
        self.elapsed = 0

    def set_loop(self, val: bool | int) -> None:
        self.animation.loop = val

    def goto_frame(self, idx: int) -> None:
        if 0 <= idx < len(self.animation.frames):
            self.frame_index = idx
            self.elapsed = 0

    def on_event(self, name: str, fn: Callable[[], None]) -> None:
        self._callbacks.setdefault(name, []).append(fn)

    def update(self, dt: int) -> None:
        if not self.playing:
            return
        if not self.animation.frames:
            return
        self.elapsed += dt
        while self.elapsed >= self.animation.frames[self.frame_index].duration:
            self.elapsed -= self.animation.frames[self.frame_index].duration
            ev = self.animation.frames[self.frame_index].event
            if ev and ev in self._callbacks:
                for cb in list(self._callbacks[ev]):
                    cb()
            self.frame_index += 1
            if self.frame_index >= len(self.animation.frames):
                if self.animation.loop is True or (isinstance(self.animation.loop, int) and self.animation.loop > 1):
                    if isinstance(self.animation.loop, int) and self.animation.loop > 1:
                        self.animation.loop -= 1
                    self.frame_index = 0
                else:
                    self.stop()
                    break

    def draw(self, x: int, y: int, scale: float = 1.0) -> None:
        draw.sprite(self.sprite, x, y, scale)
