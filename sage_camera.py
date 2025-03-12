import glm
import math

class SAGECamera:
    def __init__(self, transform):
        self.transform = transform

    def get_view_matrix(self):
        eye = self.transform.position
        direction = glm.vec3(0, 0, -1)  # Камера смотрит вдоль -Z
        rot_matrix = glm.mat4(1.0)
        rot_matrix = glm.rotate(rot_matrix, math.radians(self.transform.rotation.x), glm.vec3(1, 0, 0))
        rot_matrix = glm.rotate(rot_matrix, math.radians(self.transform.rotation.y), glm.vec3(0, 1, 0))
        rot_matrix = glm.rotate(rot_matrix, math.radians(self.transform.rotation.z), glm.vec3(0, 0, 1))
        direction = glm.mat3(rot_matrix) * direction
        target = eye + direction
        up = glm.vec3(0, 1, 0)
        return glm.lookAt(eye, target, up)