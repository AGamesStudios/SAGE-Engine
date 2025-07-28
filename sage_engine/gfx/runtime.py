from __future__ import annotations

from typing import Tuple, List, Any

from ..graphic.color import to_rgba
from ..graphic import fx
from ..graphic.state import GraphicState




class GraphicRuntime:
    def __init__(self) -> None:
        self.width = 0
        self.height = 0
        self.buffer: bytearray | None = None
        self.pitch = 0
        self.state = GraphicState()
        self._commands: List[Any] = []

    def init(self, width: int, height: int) -> None:
        """Initialize a framebuffer of the given size."""
        self.width = width
        self.height = height
        self.pitch = self.width * 4
        self.buffer = bytearray(self.height * self.pitch)


    def begin_frame(self) -> None:
        if self.buffer is not None:
            self.buffer[:] = b"\x00" * len(self.buffer)
        self._commands.clear()

    def draw_rect(self, x: int, y: int, w: int, h: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, "rect", x, y, w, h, to_rgba(color)))

    def draw_circle(self, x: int, y: int, radius: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, "circle", x, y, radius, to_rgba(color)))

    def draw_line(self, x1: int, y1: int, x2: int, y2: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, "line", x1, y1, x2, y2, to_rgba(color)))

    def add_effect(self, name: str) -> None:
        self.state.add_effect(name)

    def clear_effects(self) -> None:
        self.state.clear_effects()

    def end_frame(self) -> memoryview:
        if self.buffer is None:
            return memoryview(b"")
        for _, cmd, *args in sorted(self._commands, key=lambda c: c[0]):
            if cmd == "rect":
                self._blit_rect(*args)
            elif cmd == "circle":
                self._blit_circle(*args)
            elif cmd == "line":
                self._blit_line(*args)
        for name in self.state.effects:
            fx.apply(name, self.buffer, self.width, self.height)
        return memoryview(self.buffer)

    def shutdown(self) -> None:
        self.buffer = None

    # internal drawing helpers
    def _blit_rect(self, x: int, y: int, w: int, h: int, color: Tuple[int, int, int, int]) -> None:
        r, g, b, a = color
        for yy in range(max(0, y), min(self.height, y + h)):
            for xx in range(max(0, x), min(self.width, x + w)):
                self._blend_pixel(xx, yy, r, g, b, a)

    def _blit_circle(self, x: int, y: int, radius: int, color: Tuple[int, int, int, int]) -> None:
        r, g, b, a = color
        r2 = radius * radius
        for yy in range(max(0, y - radius), min(self.height, y + radius + 1)):
            dy = yy - y
            for xx in range(max(0, x - radius), min(self.width, x + radius + 1)):
                dx = xx - x
                if dx * dx + dy * dy <= r2:
                    self._blend_pixel(xx, yy, r, g, b, a)

    def _blit_line(self, x1: int, y1: int, x2: int, y2: int, color: Tuple[int, int, int, int]) -> None:
        r, g, b, a = color
        dx = abs(x2 - x1)
        dy = -abs(y2 - y1)
        sx = 1 if x1 < x2 else -1
        sy = 1 if y1 < y2 else -1
        err = dx + dy
        x, y = x1, y1
        while True:
            if 0 <= x < self.width and 0 <= y < self.height:
                self._blend_pixel(x, y, r, g, b, a)
            if x == x2 and y == y2:
                break
            e2 = 2 * err
            if e2 >= dy:
                err += dy
                x += sx
            if e2 <= dx:
                err += dx
                y += sy

    def _blend_pixel(self, x: int, y: int, r: int, g: int, b: int, a: int) -> None:
        off = y * self.pitch + x * 4
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
