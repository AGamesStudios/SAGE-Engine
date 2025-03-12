from pyglm import glm
import math

class SAGETransform:
    def __init__(self, position=glm.vec3(0, 0, 0), rotation=glm.vec3(0, 0, 0), scale=glm.vec3(1, 1, 1)):
        self.position = position
        self.rotation = rotation  # Ожидаем углы в градусах
        self.scale = scale

    def get_model_matrix(self):
        """Возвращает матрицу модели (T * R * S)."""
        model = glm.mat4(1.0)
        # Применяем трансляцию
        model = glm.translate(model, self.position)
        # Применяем вращение (конвертируем градусы в радианы)
        model = glm.rotate(model, math.radians(self.rotation.x), glm.vec3(1, 0, 0))  # Вокруг оси X
        model = glm.rotate(model, math.radians(self.rotation.y), glm.vec3(0, 1, 0))  # Вокруг оси Y
        model = glm.rotate(model, math.radians(self.rotation.z), glm.vec3(0, 0, 1))  # Вокруг оси Z
        # Применяем масштабирование
        model = glm.scale(model, self.scale)
        return model