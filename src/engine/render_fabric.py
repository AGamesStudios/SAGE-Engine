from __future__ import annotations

import mmap
import struct
import time
from dataclasses import dataclass, field
from typing import List, Tuple, Optional

from .smart_slice import SLICE_SIZE, SmartSliceAllocator
from .utils import TraceProfiler

__all__ = ["RenderFabric", "SpritePass"]


@dataclass
class SpritePass:
    """Render pass that batches sprites using instancing."""

    fabric: "RenderFabric"
    allocator: SmartSliceAllocator
    sprites: List[Tuple[int, Tuple[float, float, float, float]]] = field(default_factory=list)

    def add_sprite(self, atlas_index: int, transform: Tuple[float, float, float, float]) -> None:
        """Queue a sprite for drawing."""
        self.sprites.append((atlas_index, transform))

    def clear(self) -> None:
        self.sprites.clear()

    def draw(self) -> None:
        """Upload instance data and issue a single draw call."""
        if not self.sprites:
            return
        start = time.perf_counter()
        needed = (len(self.sprites) * 20 + SLICE_SIZE - 1) // SLICE_SIZE
        index = self.allocator.alloc_slice(0, needed)
        off = index * SLICE_SIZE
        slice_view = memoryview(self.allocator._mmap)[off : off + needed * SLICE_SIZE]
        for i, (tex, tr) in enumerate(self.sprites):
            struct.pack_into("<4fH", slice_view, i * 20, *tr, tex)
        if self.fabric.backend == "opengl" and self.fabric._fallback:
            if hasattr(self.fabric._fallback, "draw_sprites"):
                self.fabric._fallback.draw_sprites(self.sprites)
        if self.fabric.profiler:
            end = time.perf_counter()
            self.fabric.profiler.events.append({
                "name": "SpritePass CPU",
                "ph": "X",
                "ts": start * 1_000_000,
                "dur": (end - start) * 1_000_000,
                "pid": 0,
                "tid": 0,
            })
            self.fabric.profiler.events.append({
                "name": "GPUSubmit",
                "ph": "i",
                "ts": end * 1_000_000,
                "pid": 0,
                "tid": 0,
                "s": "g",
            })
        self.sprites.clear()

    render = draw


class RenderFabric:
    """Minimal renderer using WGPU with an OpenGL fallback."""

    def __init__(self, width: int = 1280, height: int = 720) -> None:
        self.backend = "wgpu"
        self.width = width
        self.height = height
        self.device: Optional[object] = None
        self._fallback: Optional[object] = None
        self.profiler: TraceProfiler | None = None
        self._mm = mmap.mmap(-1, SLICE_SIZE * 2048)
        self.alloc = SmartSliceAllocator(self._mm, 0, 2048)
        try:
            import wgpu.utils  # type: ignore

            self.device = wgpu.utils.get_default_device()
        except Exception:
            from .renderers.opengl_renderer import OpenGLRenderer

            self.backend = "opengl"
            self._fallback = OpenGLRenderer(width, height)
        self.sprite_pass = SpritePass(self, self.alloc)

    def present(self) -> None:  # pragma: no cover - depends on backend
        if self.backend == "opengl" and self._fallback:
            self._fallback.present()

    def close(self) -> None:  # pragma: no cover - depends on backend
        if self.backend == "opengl" and self._fallback:
            self._fallback.close()
