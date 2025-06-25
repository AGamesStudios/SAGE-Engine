from dataclasses import dataclass
import sdl2
import sdl2.ext
from PIL import Image
from engine.core.camera import Camera
from engine import units

@dataclass
class SDLRenderer:
    """2D renderer using PySDL2."""

    def __init__(self, width=640, height=480, title="SAGE 2D", resizable=True):
        if sdl2.SDL_Init(sdl2.SDL_INIT_VIDEO) != 0:
            raise RuntimeError("Failed to init SDL2")
        flags = sdl2.SDL_WINDOW_SHOWN
        if resizable:
            flags |= sdl2.SDL_WINDOW_RESIZABLE
        self.window = sdl2.SDL_CreateWindow(title.encode('utf-8'),
                                            sdl2.SDL_WINDOWPOS_CENTERED,
                                            sdl2.SDL_WINDOWPOS_CENTERED,
                                            width, height, flags)
        if not self.window:
            raise RuntimeError("Failed to create SDL window")
        self.renderer = sdl2.SDL_CreateRenderer(
            self.window, -1,
            sdl2.SDL_RENDERER_ACCELERATED | sdl2.SDL_RENDERER_PRESENTVSYNC)
        self.textures = {}
        self.width = width
        self.height = height
        self.resizable = resizable
        self._should_close = False

    def update_size(self):
        w = sdl2.Int32()
        h = sdl2.Int32()
        sdl2.SDL_GetWindowSize(self.window, w, h)
        self.width = w.value
        self.height = h.value

    def set_window_size(self, width, height):
        sdl2.SDL_SetWindowSize(self.window, width, height)
        self.update_size()

    def should_close(self):
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        r, g, b = color[:3]
        sdl2.SDL_SetRenderDrawColor(self.renderer, r, g, b, 255)
        sdl2.SDL_RenderClear(self.renderer)

    def _get_texture(self, obj):
        tex = self.textures.get(obj.image_path)
        if tex:
            return tex
        img = obj.image
        if img is None:
            img = Image.new('RGBA', (32, 32), obj.color or (255, 255, 255, 255))
        w, h = img.size
        data = img.tobytes()
        surf = sdl2.SDL_CreateRGBSurfaceFrom(data, w, h, 32, w * 4,
                0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000)
        tex = sdl2.SDL_CreateTextureFromSurface(self.renderer, surf)
        sdl2.SDL_FreeSurface(surf)
        self.textures[obj.image_path] = tex
        return tex

    def draw_scene(self, scene, camera=None):
        scale = units.UNITS_PER_METER
        zoom = 1.0
        camx = camy = 0
        camw = self.width
        camh = self.height
        if camera is not None:
            zoom = camera.zoom
            camx = camera.x * scale
            camy = camera.y * scale
            _, _, camw, camh = camera.view_rect()
        s = min(self.width / camw, self.height / camh)
        view_w = camw * s
        view_h = camh * s
        off_x = (self.width - view_w) / 2
        off_y = (self.height - view_h) / 2
        scene._sort_objects()
        for obj in scene.objects:
            if isinstance(obj, Camera):
                continue
            if camera is not None:
                x, y, w, h = obj.rect()
                left = camx - camw / 2
                top = camy - camh / 2
                if (x + w < left or x > left + camw or
                        y + h < top or y > top + camh):
                    continue
            self.draw_object(obj, camx, camy, zoom, s,
                             off_x + view_w / 2, off_y + view_h / 2)

    def draw_object(self, obj, camx=0, camy=0, zoom=1.0, scale_factor=1.0,
                    cx=None, cy=None):
        tex = self._get_texture(obj)
        scale = units.UNITS_PER_METER
        if cx is None:
            cx = self.width / 2
        if cy is None:
            cy = self.height / 2
        x = (obj.x - camx / scale) * zoom * scale * scale_factor + cx
        y = cy - (obj.y - camy / scale) * zoom * scale * scale_factor
        w = int(obj.width * obj.scale_x)
        h = int(obj.height * obj.scale_y)
        dst = sdl2.SDL_FRect(x - w / 2, y - h / 2, w, h)
        sdl2.SDL_RenderCopyExF(
            self.renderer, tex, None, dst, -obj.angle, None, sdl2.SDL_FLIP_NONE)

    def present(self):
        sdl2.SDL_RenderPresent(self.renderer)

    def close(self):
        sdl2.SDL_DestroyRenderer(self.renderer)
        sdl2.SDL_DestroyWindow(self.window)
        sdl2.SDL_Quit()
