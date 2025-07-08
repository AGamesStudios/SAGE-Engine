"""Simple SDL renderer using PySDL2."""


import ctypes
import logging
import math
from typing import Any, cast

from PIL import Image  # type: ignore[import-not-found]

from . import Renderer, register_renderer
from .opengl.drawing import parse_color
from .sdl_widget import SDLWidget
from ..mesh_utils import Mesh

try:
    import sdl2  # type: ignore[import-not-found]
except Exception as exc:  # pragma: no cover - optional dependency
    raise ImportError(
        "SDLRenderer requires PySDL2 and SDL2 libraries"
    ) from exc

logger = logging.getLogger(__name__)


class SDLRenderer(Renderer):
    """Lightweight renderer based on PySDL2."""

    def __init__(
        self,
        width: int = 640,
        height: int = 480,
        title: str = "SAGE 2D",
        *,
        widget=None,
        vsync: bool | None = None,
        keep_aspect: bool = True,
    ) -> None:
        super().__init__()
        if sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO) != 0:
            raise RuntimeError("SDL_Init failed")
        self.width = width
        self.height = height
        self.title = title
        self.vsync = vsync
        self.widget = widget
        if widget is not None and hasattr(widget, "winId"):
            handle = int(widget.winId())
            self.window = sdl2.SDL_CreateWindowFrom(ctypes.c_void_p(handle))
            self.own_window = False
            if not self.window:
                err = sdl2.SDL_GetError()
                raise RuntimeError(f"SDL_CreateWindowFrom failed: {err.decode()}")
            sdl2.SDL_SetWindowTitle(self.window, title.encode())
        else:
            self.window = sdl2.SDL_CreateWindow(
                title.encode(),
                sdl2.SDL_WINDOWPOS_CENTERED,
                sdl2.SDL_WINDOWPOS_CENTERED,
                width,
                height,
                0,
            )
            self.own_window = True
        if not self.window:
            err = sdl2.SDL_GetError()
            raise RuntimeError(f"SDL_CreateWindow failed: {err.decode()}")
        flags = 0
        if hasattr(sdl2, "SDL_RENDERER_ACCELERATED"):
            flags |= sdl2.SDL_RENDERER_ACCELERATED
        if vsync and hasattr(sdl2, "SDL_RENDERER_PRESENTVSYNC"):
            flags |= sdl2.SDL_RENDERER_PRESENTVSYNC
        self.renderer = sdl2.SDL_CreateRenderer(self.window, -1, flags)
        if not self.renderer:
            err = sdl2.SDL_GetError()
            raise RuntimeError(f"SDL_CreateRenderer failed: {err.decode()}")
        sdl2.SDL_SetRenderDrawBlendMode(self.renderer, sdl2.SDL_BLENDMODE_BLEND)
        if isinstance(self.widget, SDLWidget):
            cast(Any, self.widget).renderer = self
        self.keep_aspect = keep_aspect
        self.textures: dict[tuple[int, bool], ctypes.c_void_p] = {}

    # ------------------------------------------------------------------
    def _draw_basic_gizmo(self, gizmo, camera=None) -> None:
        """Draw a simple gizmo using SDL lines."""
        zoom = camera.zoom if camera else 1.0
        size = gizmo.size / zoom
        r, g, b, a = [int(max(0, min(255, c * 255))) for c in gizmo.color]
        sdl2.SDL_SetRenderDrawColor(self.renderer, r, g, b, a)
        cx = int(gizmo.x)
        cy = int(gizmo.y)
        if gizmo.shape.lower() == "polyline" and gizmo.vertices:
            points = [sdl2.SDL_Point(int(vx), int(vy)) for vx, vy in gizmo.vertices]
            arr = (sdl2.SDL_Point * len(points))(*points)
            sdl2.SDL_RenderDrawLines(self.renderer, arr, len(points))
            return
        if gizmo.shape.lower() == "square":
            half = int(size)
            thick = max(1, int(gizmo.thickness / zoom))
            for i in range(thick):
                rect = sdl2.SDL_Rect(
                    cx - half + i,
                    cy - half + i,
                    half * 2 - i * 2,
                    half * 2 - i * 2,
                )
                sdl2.SDL_RenderDrawRect(self.renderer, rect)
            return
        if gizmo.shape.lower() == "circle":
            steps = max(16, int(size))
            angle = 360.0 / steps
            prev_x = cx + int(size)
            prev_y = cy
            for i in range(1, steps + 1):
                ang = math.radians(i * angle)
                nx = cx + int(math.cos(ang) * size)
                ny = cy + int(math.sin(ang) * size)
                sdl2.SDL_RenderDrawLine(self.renderer, prev_x, prev_y, nx, ny)
                prev_x, prev_y = nx, ny
            return
        # cross or unknown
        half = int(size)
        thick = max(1, int(gizmo.thickness / zoom))
        for i in range(-(thick // 2), thick - thick // 2):
            sdl2.SDL_RenderDrawLine(self.renderer, cx - half, cy + i, cx + half, cy + i)
            sdl2.SDL_RenderDrawLine(self.renderer, cx + i, cy - half, cx + i, cy + half)

    # ------------------------------------------------------------------
    def clear(self, color=(0, 0, 0)) -> None:
        r, g, b = [int(c) for c in color]
        sdl2.SDL_SetRenderDrawColor(self.renderer, r, g, b, 255)
        sdl2.SDL_RenderClear(self.renderer)

    def _draw_rect(self, x: int, y: int, w: int, h: int, color) -> None:
        r, g, b, a = color
        sdl2.SDL_SetRenderDrawColor(self.renderer, r, g, b, a)
        rect = sdl2.SDL_Rect(x, y, w, h)
        sdl2.SDL_RenderFillRect(self.renderer, rect)

    def _get_texture(self, obj) -> ctypes.c_void_p:
        if obj.image is None:
            key = (0, True)
            tex = self.textures.get(key)
            if tex:
                return tex
            tex = sdl2.SDL_CreateTexture(
                self.renderer,
                sdl2.SDL_PIXELFORMAT_RGBA8888,
                sdl2.SDL_TEXTUREACCESS_STATIC,
                1,
                1,
            )
            if not tex:
                err = sdl2.SDL_GetError()
                raise RuntimeError(f"SDL_CreateTexture failed: {err.decode()}")
            arr = (ctypes.c_uint8 * 4)(255, 255, 255, 255)
            sdl2.SDL_UpdateTexture(tex, None, arr, 4)
            sdl2.SDL_SetTextureBlendMode(tex, sdl2.SDL_BLENDMODE_BLEND)
            self.textures[key] = tex
            return tex
        key = (id(obj.image), bool(getattr(obj, "smooth", True)))
        tex = self.textures.get(key)
        if tex:
            return tex
        img: Image.Image = obj.image.transpose(Image.Transpose.FLIP_TOP_BOTTOM)
        tex = sdl2.SDL_CreateTexture(
            self.renderer,
            sdl2.SDL_PIXELFORMAT_RGBA8888,
            sdl2.SDL_TEXTUREACCESS_STATIC,
            img.width,
            img.height,
        )
        if not tex:
            err = sdl2.SDL_GetError()
            raise RuntimeError(f"SDL_CreateTexture failed: {err.decode()}")
        data = img.tobytes()
        buf = (ctypes.c_uint8 * len(data)).from_buffer_copy(data)
        sdl2.SDL_UpdateTexture(tex, None, buf, img.width * 4)
        sdl2.SDL_SetTextureBlendMode(tex, sdl2.SDL_BLENDMODE_BLEND)
        self.textures[key] = tex
        return tex

    def unload_texture(self, obj) -> None:
        """Remove ``obj``'s texture from the cache."""
        key = (0, True) if obj.image is None else (id(obj.image), bool(getattr(obj, "smooth", True)))
        tex = self.textures.pop(key, None)
        if tex:
            try:
                sdl2.SDL_DestroyTexture(tex)
            except Exception:  # pragma: no cover - cleanup
                logger.exception("SDL_DestroyTexture failed")

    def _draw_sprite(self, obj) -> None:
        tex = self._get_texture(obj)
        w = int(round(obj.width * obj.scale_x))
        h = int(round(obj.height * obj.scale_y))
        rect = sdl2.SDL_Rect(
            int(round(obj.x - w * obj.pivot_x)),
            int(round(obj.y - h * obj.pivot_y)),
            w,
            h,
        )
        flip = 0
        if getattr(obj, "flip_x", False):
            flip |= sdl2.SDL_FLIP_HORIZONTAL
        if getattr(obj, "flip_y", False):
            flip |= sdl2.SDL_FLIP_VERTICAL
        sdl2.SDL_RenderCopyEx(
            self.renderer,
            tex,
            None,
            rect,
            float(getattr(obj, "angle", 0.0)),
            None,
            flip,
        )

    def _draw_mesh(self, obj, mesh: Mesh) -> None:
        rgba = parse_color(getattr(obj, "color", None))
        sdl2.SDL_SetRenderDrawColor(self.renderer, *rgba)
        scale = obj.render_scale(None)
        ang = math.radians(getattr(obj, "angle", 0.0))
        cos_a = math.cos(ang)
        sin_a = math.sin(ang)
        sx = -1.0 if getattr(obj, "flip_x", False) else 1.0
        sy = -1.0 if getattr(obj, "flip_y", False) else 1.0
        w = obj.width * obj.scale_x * scale
        h = obj.height * obj.scale_y * scale
        px = w * obj.pivot_x
        py = h * obj.pivot_y
        verts = []
        for vx, vy in mesh.vertices:
            vx = (vx * w - px) * sx + px
            vy = (vy * h - py) * sy + py
            rx = vx * cos_a - vy * sin_a
            ry = vx * sin_a + vy * cos_a
            verts.append((int(obj.x + rx), int(obj.y + ry)))
        verts.append(verts[0])
        arr = (sdl2.SDL_Point * len(verts))()
        for i, (vx, vy) in enumerate(verts):
            arr[i].x = vx
            arr[i].y = vy
        sdl2.SDL_RenderDrawLines(self.renderer, arr, len(verts))

    def _build_map_texture(self, tilemap) -> None:
        w = tilemap.width * tilemap.tile_width
        h = tilemap.height * tilemap.tile_height
        surf = sdl2.SDL_CreateRGBSurfaceWithFormat(
            0, w, h, 32, sdl2.SDL_PIXELFORMAT_RGBA8888
        )
        if not surf:
            err = sdl2.SDL_GetError()
            raise RuntimeError(f"SDL_CreateRGBSurfaceWithFormat failed: {err.decode()}")
        tw = tilemap.tile_width
        th = tilemap.tile_height
        colors = tilemap.metadata.get("colors", {})
        for y in range(tilemap.height):
            for x in range(tilemap.width):
                idx = tilemap.data[y * tilemap.width + x]
                if idx == 0:
                    continue
                rgba = parse_color(colors.get(str(idx), (200, 200, 200, 255)))
                rect = sdl2.SDL_Rect(x * tw, y * th, tw, th)
                mapped = sdl2.SDL_MapRGBA(surf.contents.format, *rgba)
                sdl2.SDL_FillRect(surf, rect, mapped)
        tex = sdl2.SDL_CreateTextureFromSurface(self.renderer, surf)
        if not tex:
            err = sdl2.SDL_GetError()
            sdl2.SDL_FreeSurface(surf)
            raise RuntimeError(f"SDL_CreateTextureFromSurface failed: {err.decode()}")
        sdl2.SDL_SetTextureBlendMode(tex, sdl2.SDL_BLENDMODE_BLEND)
        sdl2.SDL_FreeSurface(surf)
        tilemap._texture = tex

    def _draw_map(self, tilemap) -> None:
        if getattr(tilemap, "_texture", None) is None:
            self._build_map_texture(tilemap)
        rect = sdl2.SDL_Rect(
            0,
            0,
            tilemap.width * tilemap.tile_width,
            tilemap.height * tilemap.tile_height,
        )
        sdl2.SDL_RenderCopy(self.renderer, tilemap._texture, None, rect)

    def draw_scene(self, scene, camera=None, gizmos=False) -> None:  # noqa: D401
        for obj in getattr(scene, "objects", []):
            if not getattr(obj, "visible", True):
                continue
            if getattr(obj, "role", "") == "map":
                self._draw_map(obj)
                continue
            shape = getattr(obj, "shape", None)
            if isinstance(shape, str):
                shape = shape.strip().lower()
            if shape in ("rectangle", "rect", "square"):
                x = int(getattr(obj, "x", 0))
                y = int(getattr(obj, "y", 0))
                w = int(getattr(obj, "width", 32) * getattr(obj, "scale_x", 1.0))
                h = int(getattr(obj, "height", 32) * getattr(obj, "scale_y", 1.0))
                color = parse_color(getattr(obj, "color", None))
                alpha = getattr(obj, "alpha", 1.0)
                if alpha > 1.0:
                    alpha = alpha / 255.0
                final = (
                    color[0],
                    color[1],
                    color[2],
                    min(255, int(color[3] * alpha))
                )
                self._draw_rect(x, y, w, h, final)
                continue
            mesh = getattr(obj, "mesh", None)
            if isinstance(mesh, Mesh):
                self._draw_mesh(obj, mesh)
                continue
            if obj.image is not None:
                self._draw_sprite(obj)
        if gizmos or self.gizmos:
            for g in list(self.gizmos):
                self._draw_basic_gizmo(g, camera)
            self._advance_gizmos()

    def present(self) -> None:
        sdl2.SDL_RenderPresent(self.renderer)

    def close(self) -> None:
        if self.renderer:
            for tex in list(self.textures.values()):
                try:
                    sdl2.SDL_DestroyTexture(tex)
                except Exception:  # pragma: no cover - cleanup
                    logger.exception("SDL_DestroyTexture failed")
            self.textures.clear()
            sdl2.SDL_DestroyRenderer(self.renderer)
            self.renderer = None
        if self.window and getattr(self, "own_window", True):
            sdl2.SDL_DestroyWindow(self.window)
        self.window = None
        was_init = getattr(sdl2, "SDL_WasInit", lambda flag: 1)(0)
        if was_init != 0:
            sdl2.SDL_Quit()

    def reset(self) -> None:  # pragma: no cover - trivial
        pass


register_renderer("sdl", SDLRenderer)

__all__ = ["SDLRenderer"]

