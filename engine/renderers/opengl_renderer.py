from dataclasses import dataclass
from OpenGL import GL
import glfw
from PIL import Image
from engine.core.camera import Camera
from engine import units


def _ortho(left, right, bottom, top, near=-1.0, far=1.0) -> list[float]:
    """Return an orthographic projection matrix as a flat list."""
    rl = right - left
    tb = top - bottom
    fn = far - near
    return [
        2.0 / rl, 0.0, 0.0, -(right + left) / rl,
        0.0, 2.0 / tb, 0.0, -(top + bottom) / tb,
        0.0, 0.0, -2.0 / fn, -(far + near) / fn,
        0.0, 0.0, 0.0, 1.0,
    ]


@dataclass
class GLSettings:
    """Configuration options for the OpenGL renderer."""
    major: int = 2
    minor: int = 1
    vsync: bool = True

class OpenGLRenderer:
    """Basic 2D renderer using glfw and OpenGL."""

    def __init__(self, width=640, height=480, title="SAGE 2D",
                 settings: GLSettings | None = None,
                 units_per_meter: float | None = None):
        if not glfw.init():
            raise RuntimeError("Failed to init glfw")
        settings = settings or GLSettings()
        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, settings.major)
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, settings.minor)
        if units_per_meter is not None:
            units.set_units_per_meter(units_per_meter)
        self.window = glfw.create_window(width, height, title, None, None)
        if not self.window:
            glfw.terminate()
            raise RuntimeError("Failed to create window")
        glfw.make_context_current(self.window)
        glfw.swap_interval(1 if settings.vsync else 0)
        # use logical window size for coordinates but actual framebuffer size
        # for the OpenGL viewport so high-DPI displays render correctly
        winw, winh = glfw.get_window_size(self.window)
        fbw, fbh = glfw.get_framebuffer_size(self.window)
        self.width = winw
        self.height = winh
        self._setup_projection(winw, winh, fbw, fbh)
        GL.glEnable(GL.GL_TEXTURE_2D)
        self.textures = {}

    def _setup_projection(self, width, height, fbw=None, fbh=None):
        """Set the OpenGL viewport and projection."""
        if fbw is None:
            fbw = width
        if fbh is None:
            fbh = height
        GL.glViewport(0, 0, fbw, fbh)
        proj = _ortho(0.0, float(width), float(height), 0.0, -1.0, 1.0)
        GL.glMatrixMode(GL.GL_PROJECTION)
        GL.glLoadMatrixf(proj)
        GL.glMatrixMode(GL.GL_MODELVIEW)

    def update_size(self):
        """Refresh stored window and framebuffer sizes."""
        winw, winh = glfw.get_window_size(self.window)
        fbw, fbh = glfw.get_framebuffer_size(self.window)
        self.width = winw
        self.height = winh
        self._setup_projection(winw, winh, fbw, fbh)

    def set_window_size(self, width, height):
        glfw.set_window_size(self.window, width, height)
        self.update_size()

    def should_close(self):
        return glfw.window_should_close(self.window)

    def clear(self, color=(0, 0, 0)):
        r, g, b = [c / 255.0 for c in color[:3]]
        GL.glClearColor(r, g, b, 1.0)
        GL.glClear(GL.GL_COLOR_BUFFER_BIT)

    def _get_texture(self, obj):
        tex = self.textures.get(obj.image_path)
        if tex:
            return tex
        img = obj.image
        if img is None:
            img = Image.new('RGBA', (32, 32), obj.color or (255, 255, 255, 255))
        tex_id = GL.glGenTextures(1)
        GL.glBindTexture(GL.GL_TEXTURE_2D, tex_id)
        data = img.tobytes('raw', 'RGBA')
        GL.glTexImage2D(GL.GL_TEXTURE_2D, 0, GL.GL_RGBA, img.width, img.height, 0,
                        GL.GL_RGBA, GL.GL_UNSIGNED_BYTE, data)
        GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MIN_FILTER, GL.GL_NEAREST)
        GL.glTexParameteri(GL.GL_TEXTURE_2D, GL.GL_TEXTURE_MAG_FILTER, GL.GL_NEAREST)
        tex = (tex_id, img.width, img.height)
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
        GL.glPushMatrix()
        GL.glTranslatef(off_x + view_w / 2, off_y + view_h / 2, 0)
        GL.glScalef(s * zoom, s * zoom, 1)
        GL.glTranslatef(-camx, -camy, 0)
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
            self.draw_object(obj)
        GL.glPopMatrix()

    def draw_object(self, obj):
        tex_id, w, h = self._get_texture(obj)
        GL.glBindTexture(GL.GL_TEXTURE_2D, tex_id)
        GL.glPushMatrix()
        GL.glMultMatrixf(obj.transform_matrix())
        GL.glTranslatef(-w / 2, -h / 2, 0)
        if obj.color:
            GL.glColor4f(*(c/255.0 for c in obj.color))
        else:
            GL.glColor4f(1,1,1,1)
        GL.glBegin(GL.GL_QUADS)
        GL.glTexCoord2f(0, 0); GL.glVertex2f(0, 0)
        GL.glTexCoord2f(1, 0); GL.glVertex2f(w, 0)
        GL.glTexCoord2f(1, 1); GL.glVertex2f(w, h)
        GL.glTexCoord2f(0, 1); GL.glVertex2f(0, h)
        GL.glEnd()
        GL.glPopMatrix()

    def present(self):
        glfw.swap_buffers(self.window)
        glfw.poll_events()

    def close(self):
        glfw.destroy_window(self.window)
        glfw.terminate()
