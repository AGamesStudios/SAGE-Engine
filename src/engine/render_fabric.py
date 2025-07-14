from __future__ import annotations

from dataclasses import dataclass, field
from typing import List, Tuple, Optional

__all__ = ["RenderFabric", "SpritePass"]


@dataclass
class SpritePass:
    """Render pass that batches sprites using instancing."""

    fabric: "RenderFabric"
    sprites: List[Tuple[int, Tuple[float, float, float, float]]] = field(default_factory=list)

    def add_sprite(self, atlas_index: int, transform: Tuple[float, float, float, float]) -> None:
        """Queue a sprite for drawing."""
        self.sprites.append((atlas_index, transform))

    def clear(self) -> None:
        self.sprites.clear()

    def render(self) -> None:  # pragma: no cover - graphics calls depend on backend
        if self.fabric.backend == "opengl":
            # Delegate to the fallback renderer if WGPU is unavailable
            return
        # Real implementation would upload instance data and draw
        pass


class RenderFabric:
    """Minimal renderer using WGPU with an OpenGL fallback."""

    def __init__(self, width: int = 1280, height: int = 720) -> None:
        self.backend = "wgpu"
        self.width = width
        self.height = height
        self.device: Optional[object] = None
        self._fallback: Optional[object] = None
        try:
            import wgpu.utils  # type: ignore

            self.device = wgpu.utils.get_default_device()
        except Exception:
            from .renderers.opengl_renderer import OpenGLRenderer

            self.backend = "opengl"
            self._fallback = OpenGLRenderer(width, height)
        self.sprite_pass = SpritePass(self)

    def present(self) -> None:  # pragma: no cover - depends on backend
        if self.backend == "opengl" and self._fallback:
            self._fallback.present()

    def close(self) -> None:  # pragma: no cover - depends on backend
        if self.backend == "opengl" and self._fallback:
            self._fallback.close()
