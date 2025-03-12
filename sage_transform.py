import glm
import math

class SAGETransform:
    def __init__(self, position=glm.vec3(0, 0, 0), rotation=glm.vec3(0, 0, 0), scale=glm.vec3(1, 1, 1)):
        self.position = position
        self.rotation = rotation  # в градусах
        self.scale = scale

    def get_model_matrix(self):
        model = glm.mat4(1.0)
        model = glm.translate(model, self.position)
        model = glm.rotate(model, math.radians(self.rotation.x), glm.vec3(1, 0, 0))
        model = glm.rotate(model, math.radians(self.rotation.y), glm.vec3(0, 1, 0))
        model = glm.rotate(model, math.radians(self.rotation.z), glm.vec3(0, 0, 1))
        model = glm.scale(model, self.scale)
        return model