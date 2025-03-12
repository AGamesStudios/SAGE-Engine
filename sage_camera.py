from pyglm import glm

class SAGECamera:
    def __init__(self, transform):
        self.transform = transform

    def get_view_matrix(self):
        """Возвращает матрицу вида."""
        position = self.transform.position
        forward = glm.normalize(glm.vec3(0, 0, 0) - position)
        up = glm.vec3(0, 1, 0)
        return glm.lookAt(position, position + forward, up)