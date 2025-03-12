import glfw
from OpenGL.GL import *
import glm

class SAGEWindow:
    def __init__(self, width, height, title):
        if not glfw.init():
            raise Exception("Не удалось инициализировать GLFW")
        self.window = glfw.create_window(width, height, title, None, None)
        if not self.window:
            glfw.terminate()
            raise Exception("Не удалось создать окно")
        glfw.make_context_current(self.window)
        glfw.set_window_size_callback(self.window, self._resize_callback)
        self.width = width
        self.height = height
        self._update_projection()
        glClearColor(0.2, 0.3, 0.3, 1.0)
        glEnable(GL_DEPTH_TEST)

    def _resize_callback(self, window, width, height):
        glViewport(0, 0, width, height)
        self.width = width
        self.height = height
        self._update_projection()

    def _update_projection(self):
        aspect = self.width / self.height
        self._projection = glm.perspective(glm.radians(45.0), aspect, 0.1, 100.0)

    def get_projection(self):
        return self._projection

    def poll_events(self):
        glfw.poll_events()
        if glfw.get_key(self.window, glfw.KEY_ESCAPE) == glfw.PRESS:
            glfw.set_window_should_close(self.window, True)

    def should_close(self):
        return glfw.window_should_close(self.window)

    def swap_buffers(self):
        glfw.swap_buffers(self.window)

    def terminate(self):
        glfw.terminate()