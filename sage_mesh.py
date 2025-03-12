import pywavefront
import numpy as np
from OpenGL.GL import *

class SAGEMesh:
    def __init__(self, obj_file):
        scene = pywavefront.Wavefront(obj_file, collect_faces=True)
        positions = []
        for mesh in scene.mesh_list:
            for face in mesh.faces:
                for vertex_idx in face:
                    vertex = scene.vertices[vertex_idx]
                    positions.extend(vertex[0:3])  # Только позиции
        self.positions = np.array(positions, dtype=np.float32)
        self.indices = np.array([i for i in range(len(positions) // 3)], dtype=np.uint32)
        self.vao = glGenVertexArrays(1)
        self.vbo = glGenBuffers(1)
        self.ebo = glGenBuffers(1)
        self._setup_mesh()

    def _setup_mesh(self):
        glBindVertexArray(self.vao)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, self.positions.nbytes, self.positions, GL_STATIC_DRAW)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, None)
        glEnableVertexAttribArray(0)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, self.ebo)
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, self.indices.nbytes, self.indices, GL_STATIC_DRAW)
        glBindVertexArray(0)

    def draw(self):
        glBindVertexArray(self.vao)
        glDrawElements(GL_TRIANGLES, len(self.indices), GL_UNSIGNED_INT, None)
        glBindVertexArray(0)