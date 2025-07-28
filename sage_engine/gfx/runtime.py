from __future__ import annotations

from typing import Tuple, List, Any, Iterable

from ..graphic.color import to_rgba
from ..graphic import fx
from ..graphic.state import GraphicState
from ..logger import logger




class GraphicRuntime:
    def __init__(self) -> None:
        self.width = 0
        self.height = 0
        self.buffer: bytearray | None = None
        self.pitch = 0
        self.state = GraphicState()
        self._stack: List[tuple[int, tuple[int, int, int, int], list[str]]] = []
        self._commands: List[Any] = []
        self.clear_color: tuple[int, int, int, int] = (0, 0, 0, 255)

    def init(self, width: int, height: int) -> None:
        """Initialize a framebuffer of the given size."""
        self.width = width
        self.height = height
        self.pitch = self.width * 4
        self.buffer = bytearray(self.height * self.pitch)


    def begin_frame(self, color=None) -> None:
        if color is not None:
            self.clear_color = to_rgba(color)
        if self.buffer is not None:
            r, g, b, a = self.clear_color
            row = bytes((b, g, r, a)) * self.width
            for y in range(self.height):
                start = y * self.pitch
                self.buffer[start:start+self.pitch] = row
        logger.debug("begin_frame clear=%s", self.clear_color, tag="gfx")
        self._commands.clear()
        self._stack.clear()

    def push_state(self) -> None:
        self._stack.append(self.state.snapshot())

    def pop_state(self) -> None:
        if self._stack:
            self.state.restore(self._stack.pop())

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

    def draw_polygon(self, points: Iterable[tuple[int, int]], color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, "polygon", list(points), to_rgba(color)))

    def draw_rounded_rect(self, x: int, y: int, w: int, h: int, radius: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, "rounded_rect", x, y, w, h, radius, to_rgba(color)))

    def draw_text(self, *args, **kwargs) -> None:
        pass

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
            elif cmd == "polygon":
                self._blit_polygon(args[0], args[1])
            elif cmd == "rounded_rect":
                self._blit_rounded_rect(*args)
        for name in self.state.effects:
            fx.apply(name, self.buffer, self.width, self.height)
        logger.debug("end_frame %d commands", len(self._commands), tag="gfx")
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

    def _blit_polygon(self, points: List[tuple[int, int]], color: Tuple[int, int, int, int]) -> None:
        if len(points) < 3:
            return
        # simple polygon fill using bounding box and winding
        xs, ys = zip(*points)
        min_x, max_x = max(min(xs), 0), min(max(xs), self.width - 1)
        min_y, max_y = max(min(ys), 0), min(max(ys), self.height - 1)
        r, g, b, a = color
        for y in range(min_y, max_y + 1):
            crossings = []
            for i in range(len(points)):
                x1, y1 = points[i]
                x2, y2 = points[(i + 1) % len(points)]
                if (y1 <= y < y2) or (y2 <= y < y1):
                    t = (y - y1) / (y2 - y1)
                    crossings.append(int(x1 + t * (x2 - x1)))
            crossings.sort()
            for i in range(0, len(crossings), 2):
                x_start = max(crossings[i], min_x)
                x_end = min(crossings[i+1], max_x) if i+1 < len(crossings) else x_start
                for x in range(x_start, x_end + 1):
                    self._blend_pixel(x, y, r, g, b, a)

    def _blit_rounded_rect(self, x: int, y: int, w: int, h: int, radius: int, color: Tuple[int, int, int, int]) -> None:
        r, g, b, a = color
        for yy in range(h):
            for xx in range(w):
                rx = min(xx, w - xx - 1)
                ry = min(yy, h - yy - 1)
                if rx*rx + ry*ry <= radius*radius:
                    px = x + xx
                    py = y + yy
                    if 0 <= px < self.width and 0 <= py < self.height:
                        self._blend_pixel(px, py, r, g, b, a)

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
