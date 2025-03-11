import glfw
from OpenGL.GL import *
from OpenGL.GL.shaders import compileProgram, compileShader
import numpy as np
from pyglm import glm  # Исправленный импорт
import ctypes
import pygame

# Инициализация Pygame для аудио
pygame.mixer.init()


# Класс окна
class SAGEWindow:
    def __init__(self, width=800, height=600, title="SAGE Engine"):
        if not glfw.init():
            raise Exception("Не удалось инициализировать GLFW")
        self.window = glfw.create_window(width, height, title, None, None)
        if not self.window:
            glfw.terminate()
            raise Exception("Не удалось создать окно")
        glfw.make_context_current(self.window)
        glfw.set_window_size_callback(self.window, self._resize_callback)

    def _resize_callback(self, window, width, height):
        glViewport(0, 0, width, height)

    def should_close(self):
        return glfw.window_should_close(self.window)

    def poll_events(self):
        glfw.poll_events()
        if glfw.get_key(self.window, glfw.KEY_ESCAPE) == glfw.PRESS:
            glfw.set_window_should_close(self.window, True)

    def swap_buffers(self):
        glfw.swap_buffers(self.window)

    def get_framebuffer_size(self):
        return glfw.get_framebuffer_size(self.window)

    def terminate(self):
        glfw.terminate()


# Класс шейдеров
class SAGEShader:
    def __init__(self, vertex_source, fragment_source):
        self.program = compileProgram(
            compileShader(vertex_source, GL_VERTEX_SHADER),
            compileShader(fragment_source, GL_FRAGMENT_SHADER)
        )

    def use(self):
        glUseProgram(self.program)

    def set_mat4(self, name, matrix):
        glUniformMatrix4fv(glGetUniformLocation(self.program, name), 1, GL_FALSE, glm.value_ptr(matrix))

    def cleanup(self):
        glDeleteProgram(self.program)


# Класс меша
class SAGEMesh:
    def __init__(self, vertices, indices):
        self.vertices = vertices
        self.indices = indices
        self._setup_mesh()

    def _setup_mesh(self):
        self.vao = glGenVertexArrays(1)
        self.vbo = glGenBuffers(1)
        self.ebo = glGenBuffers(1)
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, self.vertices.nbytes, self.vertices, GL_STATIC_DRAW)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.ebo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, self.indices.nbytes, self.indices, GL_STATIC_DRAW)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), ctypes.c_void_p(0))
        glEnableVertexAttribArray(0)
        glBindVertexArray(0)

    def draw(self):
        glBindVertexArray(self.vao)
        glDrawElements(GL_TRIANGLES, len(self.indices), GL_UNSIGNED_INT, None)
        glBindVertexArray(0)

    def cleanup(self):
        glDeleteVertexArrays(1, [self.vao])
        glDeleteBuffers(1, [self.vbo])
        glDeleteBuffers(1, [self.ebo])


# Класс рендеринга
class SAGERenderer:
    def __init__(self):
        glClearColor(0.1, 0.1, 0.1, 1.0)
        glEnable(GL_DEPTH_TEST)

    def clear(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

    def render(self, mesh, shader, model, view, projection):
        shader.use()
        shader.set_mat4("model", model)
        shader.set_mat4("view", view)
        shader.set_mat4("projection", projection)
        mesh.draw()


# Класс UI
class SAGEUI:
    def draw_text(self, text, x, y):
        print(f"Отрисовка текста '{text}' в позиции ({x}, {y})")


# Класс аудио
class SAGEAudio:
    def play_sound(self, sound_file):
        sound = pygame.mixer.Sound(sound_file)
        sound.play()


# Основной класс движка
class SAGEEngine:
    def __init__(self):
        self.window = SAGEWindow()
        self.renderer = SAGERenderer()
        self.ui = SAGEUI()
        self.audio = SAGEAudio()

        # Шейдеры
        vertex_shader = """
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
        """
        fragment_shader = """
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0); // Белый цвет
        }
        """
        self.shader = SAGEShader(vertex_shader, fragment_shader)

        # Тестовый куб
        vertices = np.array([-0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5,
                             -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5], dtype=np.float32)
        indices = np.array([0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 0, 1, 5, 5, 4, 0,
                            2, 3, 7, 7, 6, 2, 0, 3, 7, 7, 4, 0, 1, 2, 6, 6, 5, 1], dtype=np.uint32)
        self.cube = SAGEMesh(vertices, indices)

        self.view = glm.lookAt(glm.vec3(0, 0, 5), glm.vec3(0, 0, 0), glm.vec3(0, 1, 0))

    def run(self):
        while not self.window.should_close():
            self.window.poll_events()
            self.renderer.clear()

            width, height = self.window.get_framebuffer_size()
            projection = glm.perspective(glm.radians(45.0), width / height, 0.1, 100.0)
            model = glm.rotate(glm.mat4(1.0), glfw.get_time(), glm.vec3(0, 1, 0))

            self.renderer.render(self.cube, self.shader, model, self.view, projection)
            self.ui.draw_text("SAGE Engine", 10, 10)
            self.window.swap_buffers()

        self.cube.cleanup()
        self.shader.cleanup()
        self.window.terminate()


# Запуск
if __name__ == "__main__":
    engine = SAGEEngine()
    engine.run()