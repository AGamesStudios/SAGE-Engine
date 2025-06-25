from dataclasses import dataclass
import pygame
from PIL import Image
from engine.core.camera import Camera
from engine import units


@dataclass
class PygameRenderer:
    """Simple renderer using pygame."""

    def __init__(self, width=640, height=480, title="SAGE 2D", *, surface=None, resizable=True):
        pygame.init()
        self.resizable = resizable
        if surface is None:
            flags = pygame.HWSURFACE | pygame.DOUBLEBUF
            if resizable:
                flags |= pygame.RESIZABLE
            self.surface = pygame.display.set_mode((width, height), flags)
            pygame.display.set_caption(title)
            self.window = pygame.display.get_surface()
        else:
            self.surface = surface
            self.window = None
        self.width, self.height = self.surface.get_size()
        self.textures = {}
        self._cache = {}
        self._should_close = False

    def update_size(self):
        self.width, self.height = self.surface.get_size()

    def set_window_size(self, width, height):
        if self.window:
            flags = pygame.HWSURFACE | pygame.DOUBLEBUF
            if self.resizable:
                flags |= pygame.RESIZABLE
            self.surface = pygame.display.set_mode((width, height), flags)
        else:
            self.surface = pygame.Surface((width, height))
        self.update_size()

    def should_close(self):
        return self._should_close

    def clear(self, color=(0, 0, 0)):
        self.surface.fill(color)

    def _get_texture(self, obj):
        tex = self.textures.get(obj.image_path)
        if tex:
            return tex
        img = obj.image
        if img is None:
            img = Image.new('RGBA', (32, 32), obj.color or (255, 255, 255, 255))
        mode = img.mode
        size = img.size
        data = img.tobytes()
        tex = pygame.image.frombuffer(data, size, mode)
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
            camw = camera.width * scale
            camh = camera.height * scale
        s = min(self.width / camw, self.height / camh)
        view_w = camw * s
        view_h = camh * s
        off_x = (self.width - view_w) / 2
        off_y = (self.height - view_h) / 2
        camw /= zoom
        camh /= zoom
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
            self.draw_object(obj, camx, camy, zoom, s, off_x + view_w / 2, off_y + view_h / 2)

    def draw_object(self, obj, camx=0, camy=0, zoom=1.0, scale_factor=1.0, cx=None, cy=None):
        tex = self._get_texture(obj)
        key = id(obj)
        info = (obj.scale_x, obj.scale_y, obj.angle)
        cached = self._cache.get(key)
        if cached and cached[1] == info:
            surf = cached[0]
        else:
            surf = tex
            w = int(obj.width * obj.scale_x)
            h = int(obj.height * obj.scale_y)
            if w != obj.width or h != obj.height:
                surf = pygame.transform.scale(surf, (w, h))
            if obj.angle:
                surf = pygame.transform.rotate(surf, -obj.angle)
            self._cache[key] = (surf, info)
        scale = units.UNITS_PER_METER
        if cx is None:
            cx = self.width / 2
        if cy is None:
            cy = self.height / 2
        x = (obj.x - camx / scale) * zoom * scale * scale_factor + cx
        y = (obj.y - camy / scale) * zoom * scale * scale_factor + cy
        rect = surf.get_rect(center=(int(x), int(y)))
        self.surface.blit(surf, rect)

    def present(self):
        if self.window:
            pygame.display.flip()

    def close(self):
        if self.window:
            pygame.quit()
