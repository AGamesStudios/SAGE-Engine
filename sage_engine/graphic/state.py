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

    def add_effect(self, name: str) -> None:
        if name not in self.effects:
            self.effects.append(name)

    def clear_effects(self) -> None:
        self.effects.clear()
