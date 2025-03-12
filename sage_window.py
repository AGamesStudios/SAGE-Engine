import glfw
from OpenGL.GL import *
from pyglm import glm

class SAGEWindow:
    def __init__(self, config):
        if not glfw.init():
            raise Exception("Не удалось инициализировать GLFW")
        self.window = glfw.create_window(config['width'], config['height'], config['title'], None, None)
        if not self.window:
            glfw.terminate()
            raise Exception("Не удалось создать окно")
        glfw.make_context_current(self.window)
        glfw.set_window_size_callback(self.window, self._resize_callback)
        self._projection = glm.perspective(glm.radians(45.0), config['width'] / config['height'], 0.1, 100.0)
        self.width = config['width']
        self.height = config['height']

    def _resize_callback(self, window, width, height):
        """Обновляет область просмотра и проекцию при изменении размера окна."""
        glViewport(0, 0, width, height)
        self._projection = glm.perspective(glm.radians(45.0), width / height, 0.1, 100.0)
        self.width = width
        self.height = height

    def should_close(self):
        return glfw.window_should_close(self.window)

    def poll_events(self):
        glfw.poll_events()
        if glfw.get_key(self.window, glfw.KEY_ESCAPE) == glfw.PRESS:
            glfw.set_window_should_close(self.window, True)

    def swap_buffers(self):
        glfw.swap_buffers(self.window)

    def get_projection(self):
        """Возвращает матрицу проекции."""
        return self._projection

    def terminate(self):
        glfw.terminate()