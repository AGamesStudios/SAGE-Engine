from __future__ import annotations

class GraphicState:
    """Current drawing state for :mod:`sage_engine.gfx`."""

    def __init__(self) -> None:
        #: Z index used for newly drawn primitives
        self.z = 0
        #: Default drawing color (RGBA)
        self.color = (255, 255, 255, 255)
        #: List of framebuffer effects to apply in :func:`gfx.end_frame`
        self.effects: list[str] = []

    def snapshot(self) -> tuple[int, tuple[int, int, int, int], list[str]]:
        """Return a serializable snapshot of this state."""
        return self.z, self.color, list(self.effects)

    def restore(self, snap: tuple[int, tuple[int, int, int, int], list[str]]) -> None:
        """Restore state from :func:`snapshot`."""
        self.z, self.color, eff = snap
        self.effects = list(eff)

    def add_effect(self, name: str) -> None:
        if name not in self.effects:
            self.effects.append(name)

    def clear_effects(self) -> None:
        self.effects.clear()


PixelFormat = ("BGRA8", "premultiplied")
