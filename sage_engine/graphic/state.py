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


class GraphicConfig:
    """Configuration parameters loaded from ``engine.sagecfg``."""

    def __init__(self) -> None:
        self.antialiasing = "None"
        self.filtering = "nearest"
        self.dynamic_resolution = False
        self.gamma_correction = False
        self.style = "default"
        self.fallback_mode = "None"

    def load_from_dict(self, cfg: dict) -> None:
        self.antialiasing = cfg.get("antialiasing", self.antialiasing)
        self.filtering = cfg.get("filtering", self.filtering)
        self.dynamic_resolution = bool(cfg.get("dynamic_resolution", self.dynamic_resolution))
        self.gamma_correction = bool(cfg.get("gamma_correction", self.gamma_correction))
        self.style = cfg.get("style", self.style)
        self.fallback_mode = cfg.get("fallback_mode", self.fallback_mode)


config = GraphicConfig()


# Module-level graphic state storage for quick access to configurable parameters
_graphic_state: dict[str, object] = {}

def set_state(key: str, value) -> None:
    """Set a graphic parameter by key."""
    _graphic_state[key] = value


def get_state(key: str, default=None):
    """Retrieve a graphic parameter or default."""
    return _graphic_state.get(key, default)


def clear_state() -> None:
    """Reset all stored graphic parameters."""
    _graphic_state.clear()


def export_state() -> dict:
    """Return a copy of the stored graphic state."""
    return dict(_graphic_state)
