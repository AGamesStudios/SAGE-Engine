import numpy as np
from OpenGL.GL import *
import ctypes

class SAGEMesh:
    def __init__(self, vertices, indices, tex_coords=None, normals=None):
        self.vertices = np.array(vertices, dtype=np.float32)
        self.indices = np.array(indices, dtype=np.uint32)
        self.tex_coords = np.array(tex_coords, dtype=np.float32) if tex_coords else None
        self.normals = np.array(normals, dtype=np.float32) if normals else None
        self._setup_mesh()

    def _setup_mesh(self):
        """Настраивает буферы OpenGL с interleaved атрибутами."""
        # Подготовка данных
        vertex_data = [self.vertices]
        stride = 3 * sizeof(GLfloat)  # Начальное значение для позиций
        if self.tex_coords is not None:
            vertex_data.append(self.tex_coords)
            stride += 2 * sizeof(GLfloat)
        if self.normals is not None:
            vertex_data.append(self.normals)
            stride += 3 * sizeof(GLfloat)

        # Создание VBO
        self.vao = glGenVertexArrays(1)
        self.vbo = glGenBuffers(1)
        self.ebo = glGenBuffers(1)
        glBindVertexArray(self.vao)

        # Interleaved vertex data
        interleaved_data = np.hstack(vertex_data) if len(vertex_data) > 1 else self.vertices
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, interleaved_data.nbytes, interleaved_data, GL_STATIC_DRAW)

        # Индексы
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.ebo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, self.indices.nbytes, self.indices, GL_STATIC_DRAW)

        # Настройка атрибутов
        offset = 0
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, ctypes.c_void_p(offset))
        glEnableVertexAttribArray(0)
        offset += 3 * sizeof(GLfloat)

        if self.tex_coords is not None:
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, ctypes.c_void_p(offset))
            glEnableVertexAttribArray(1)
            offset += 2 * sizeof(GLfloat)

        if self.normals is not None:
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, ctypes.c_void_p(offset))
            glEnableVertexAttribArray(2)

        glBindVertexArray(0)

    def draw(self):
        """Отрисовывает меш."""
        glBindVertexArray(self.vao)
        glDrawElements(GL_TRIANGLES, len(self.indices), GL_UNSIGNED_INT, None)
        glBindVertexArray(0)

    def cleanup(self):
        """Очищает ресурсы меша."""
        glDeleteVertexArrays(1, [self.vao])
        glDeleteBuffers(1, [self.vbo])
        glDeleteBuffers(1, [self.ebo])