from OpenGL import GL
import glfw
from PIL import Image

class OpenGLRenderer:
    """Basic 2D renderer using glfw and OpenGL."""

    def __init__(self, width=640, height=480, title="SAGE 2D"):
        if not glfw.init():
            raise RuntimeError("Failed to init glfw")
        self.window = glfw.create_window(width, height, title, None, None)
        if not self.window:
            glfw.terminate()
            raise RuntimeError("Failed to create window")
        glfw.make_context_current(self.window)
        self.width = width
        self.height = height
        GL.glViewport(0, 0, width, height)
        GL.glMatrixMode(GL.GL_PROJECTION)
        GL.glLoadIdentity()
        GL.glOrtho(0, width, height, 0, -1, 1)
        GL.glMatrixMode(GL.GL_MODELVIEW)
        GL.glEnable(GL.GL_TEXTURE_2D)
        self.textures = {}

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

    def draw_scene(self, scene):
        for obj in sorted(scene.objects, key=lambda o: getattr(o, 'z', 0)):
            self.draw_object(obj)

    def draw_object(self, obj):
        tex_id, w, h = self._get_texture(obj)
        GL.glBindTexture(GL.GL_TEXTURE_2D, tex_id)
        GL.glPushMatrix()
        GL.glTranslatef(obj.x, obj.y, 0)
        GL.glTranslatef(w/2, h/2, 0)
        GL.glRotatef(-obj.angle, 0, 0, 1)
        GL.glScalef(obj.scale_x, obj.scale_y, 1)
        GL.glTranslatef(-w/2, -h/2, 0)
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
