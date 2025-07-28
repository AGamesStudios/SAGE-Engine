from __future__ import annotations

from typing import Tuple

from ..graphic.color import to_rgba
from ..graphic import fx




class GraphicRuntime:
    def __init__(self) -> None:
        self.width = 0
        self.height = 0
        self.buffer: bytearray | None = None
        self.pitch = 0
        self.effects: list[str] = []

    def init(self, width: int, height: int) -> None:
        """Initialize a framebuffer of the given size."""
        self.width = width
        self.height = height
        self.pitch = self.width * 4
        self.buffer = bytearray(self.height * self.pitch)


    def begin_frame(self) -> None:
        if self.buffer is not None:
            self.buffer[:] = b"\x00" * len(self.buffer)

    def draw_rect(self, x: int, y: int, w: int, h: int, color) -> None:
        if self.buffer is None:
            return
        r, g, b, a = to_rgba(color)
        for yy in range(max(0, y), min(self.height, y + h)):
            for xx in range(max(0, x), min(self.width, x + w)):
                off = yy * self.pitch + xx * 4
                if a == 255:
                    self.buffer[off:off+4] = bytes((b, g, r, 255))
                else:
                    db = self.buffer[off]
                    dg = self.buffer[off + 1]
                    dr = self.buffer[off + 2]
                    da = self.buffer[off + 3]
                    na = a + da * (255 - a) // 255
                    nb = (b * a + db * (255 - a)) // 255
                    ng = (g * a + dg * (255 - a)) // 255
                    nr = (r * a + dr * (255 - a)) // 255
                    self.buffer[off:off+4] = bytes((nb, ng, nr, na))

    def add_effect(self, name: str) -> None:
        if name not in self.effects:
            self.effects.append(name)

    def clear_effects(self) -> None:
        self.effects.clear()

    def end_frame(self) -> memoryview:
        for name in self.effects:
            fx.apply(name, self.buffer, self.width, self.height)
        return memoryview(self.buffer)

    def shutdown(self) -> None:
        self.buffer = None
