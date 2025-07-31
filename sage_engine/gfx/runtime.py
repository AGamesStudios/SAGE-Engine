from __future__ import annotations

from typing import Tuple, List, Any, Iterable

from .. import core

from ..graphic.color import to_premul_rgba, to_bgra8_premul
from ..graphic import fx
from ..graphic.state import GraphicState
from ..render import rustbridge as rust
from ctypes import c_void_p, c_int, c_uint
import ctypes
from ..logger import logger

BYTES_PER_PIXEL = 4




class GraphicRuntime:
    def __init__(self) -> None:
        self.width = 0
        self.height = 0
        self.buffer: bytearray | None = None
        self.pitch = 0
        self.state = GraphicState()
        self._stack: List[tuple[int, tuple[int, int, int, int], list[str]]] = []
        self._commands: List[Any] = []
        self._seq_counter = 0
        self.clear_color: int = to_bgra8_premul((0, 0, 0, 255))
        self.auto_resize = True
        self._frame_counter = 0
        self._last_error_key: str | None = None
        self._last_error_frame: int = -1
        self._error_interval = 60


    def init(self, width: int, height: int) -> None:
        """Initialize a framebuffer of the given size."""
        # framebuffer will be resized on demand in flush_frame
        from ..events import on
        on("window_resized", lambda w, h: self.set_dimensions(w, h))
        self.width = width
        self.height = height
        self.pitch = self.width * BYTES_PER_PIXEL
        self.buffer = bytearray(self.height * self.pitch)

    def set_dimensions(self, width: int, height: int) -> None:
        """Update dimensions and reallocate the buffer."""
        self.realloc_buffer(width, height)

    def realloc_buffer(self, width: int, height: int) -> None:
        """Reallocate the internal framebuffer to the given size."""
        if width <= 0 or height <= 0:
            return
        if width == self.width and height == self.height:
            return
        self.width = width
        self.height = height
        self.pitch = self.width * BYTES_PER_PIXEL
        self.buffer = bytearray(self.height * self.pitch)


    def begin_frame(self, color=None) -> None:
        """Start drawing a new frame."""
        w, h = self.width or 1, self.height or 1
        if self.buffer is None:
            self.init(w, h)
            logger.debug("[gfx] Framebuffer allocated %dx%d", w, h)
        if color is not None:
            self.clear_color = to_bgra8_premul(color)
        if self.buffer is not None:
            addr = ctypes.addressof(ctypes.c_char.from_buffer(self.buffer))
            rust.lib.sage_clear(
                c_void_p(addr),
                c_int(self.width),
                c_int(self.height),
                c_uint(self.clear_color),
            )
        logger.debug("begin_frame clear=%s", self.clear_color, tag="gfx")
        self._commands.clear()
        self._stack.clear()
        self._seq_counter = 0

    def _ensure_buffer_size(self) -> None:
        """Ensure the framebuffer matches ``self.width`` and ``self.height``."""
        if self.width <= 0 or self.height <= 0:
            return
        expected_size = self.width * self.height * BYTES_PER_PIXEL
        actual_size = len(self.buffer) if self.buffer is not None else 0
        logger.debug(
            "[gfx] Validating framebuffer: %d bytes vs expected %d",
            actual_size,
            expected_size,
        )
        if actual_size != expected_size:
            if self.auto_resize:
                self.realloc_buffer(self.width, self.height)
                logger.info(
                    "[gfx] Framebuffer reallocated due to resize: %dx%d",
                    self.width,
                    self.height,
                )
            else:
                key = f"{self.width}x{self.height}"
                if (
                    self._last_error_key != key
                    or self._frame_counter - self._last_error_frame >= self._error_interval
                ):
                    logger.error(
                        "[gfx] Buffer mismatch: fb %dx%d bytes vs expected %dx%d",
                        actual_size,
                        expected_size,
                        self.width,
                        self.height,
                    )
                    self._last_error_key = key
                    self._last_error_frame = self._frame_counter

    def push_state(self) -> None:
        self._stack.append(self.state.snapshot())

    def pop_state(self) -> None:
        if self._stack:
            self.state.restore(self._stack.pop())

    def draw_rect(self, x: int, y: int, w: int, h: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, self._seq_counter, "rect", x, y, w, h, to_premul_rgba(color)))
        self._seq_counter += 1

    def draw_circle(self, x: int, y: int, radius: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, self._seq_counter, "circle", x, y, radius, to_premul_rgba(color)))
        self._seq_counter += 1

    def draw_line(self, x1: int, y1: int, x2: int, y2: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, self._seq_counter, "line", x1, y1, x2, y2, to_premul_rgba(color)))
        self._seq_counter += 1

    def draw_polygon(self, points: Iterable[tuple[int, int]], color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, self._seq_counter, "polygon", list(points), to_premul_rgba(color)))
        self._seq_counter += 1

    def draw_rounded_rect(self, x: int, y: int, w: int, h: int, radius: int, color=None, z: int | None = None) -> None:
        color = color if color is not None else self.state.color
        z = self.state.z if z is None else z
        self._commands.append((z, self._seq_counter, "rounded_rect", x, y, w, h, radius, to_premul_rgba(color)))
        self._seq_counter += 1

    def draw_text(self, *args, **kwargs) -> None:
        pass

    def add_effect(self, name: str) -> None:
        self.state.add_effect(name)

    def clear_effects(self) -> None:
        self.state.clear_effects()

    def end_frame(self) -> memoryview:
        if self.buffer is None:
            return memoryview(b"")
        for _, _, cmd, *args in sorted(self._commands, key=lambda c: (c[0], c[1])):
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

    def flush_frame(self, handle: Any | None = None, fsync: Any | None = None) -> None:
        """Finish drawing and present the buffer via :mod:`render`."""
        render = core.get("render")
        if render is None:
            return
        self._ensure_buffer_size()
        buf = self.end_frame()
        expected_size = self.width * self.height * BYTES_PER_PIXEL
        actual_size = len(buf)
        if actual_size != expected_size:
            self.realloc_buffer(self.width, self.height)
            logger.info("[gfx] Framebuffer reallocated to %dx%d", self.width, self.height)
            self.begin_frame()
            buf = self.end_frame()
        self._frame_counter += 1
        backend = render._get_backend()
        ctx = None
        if backend and hasattr(backend, "_contexts"):
            h = int(handle or getattr(render._get_context(), "output_target", 0) or 0)
            ctx = backend._contexts.get(h)
        if ctx:
            expected_size = ctx.stride * ctx.height
            actual_size = len(buf)
            if actual_size < expected_size:
                logger.error(
                    "[gfx] Buffer size %d < expected %d", actual_size, expected_size
                )
                return
        logger.debug("[gfx] Framebuffer size: %d bytes", len(buf))
        logger.debug("[gfx] Sending to render: handle=%s", handle)
        render.present(buf, handle)
        if fsync is not None and hasattr(fsync, "sleep_until_next_frame"):
            fsync.sleep_until_next_frame()
        # clear commands after presenting to avoid memory growth
        self._commands.clear()
        self._stack.clear()

    def shutdown(self) -> None:
        self.buffer = None

    # internal drawing helpers
    def _blit_rect(self, x: int, y: int, w: int, h: int, color: Tuple[int, int, int, int]) -> None:
        if self.buffer is None:
            return
        c = (color[0] << 16) | (color[1] << 8) | color[2] | (color[3] << 24)
        addr = c_void_p(ctypes.addressof(ctypes.c_char.from_buffer(self.buffer)))
        rust.lib.sage_draw_rect(
            addr,
            c_int(self.width),
            c_int(self.height),
            c_int(self.pitch),
            c_int(x),
            c_int(y),
            c_int(w),
            c_int(h),
            c_uint(c),
        )

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
        off = y * self.pitch + x * BYTES_PER_PIXEL
        if a == 255:
            self.buffer[off:off+BYTES_PER_PIXEL] = bytes((b, g, r, 255))
        elif a == 0:
            return
        else:
            db = self.buffer[off]
            dg = self.buffer[off + 1]
            dr = self.buffer[off + 2]
            da = self.buffer[off + 3]
            inv = 255 - a
            nb = b + (db * inv // 255)
            ng = g + (dg * inv // 255)
            nr = r + (dr * inv // 255)
            na = a + (da * inv // 255)
            self.buffer[off:off+BYTES_PER_PIXEL] = bytes((nb, ng, nr, na))
