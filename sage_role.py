from pyglm import glm
from sage_transform import SAGETransform
from sage_camera import SAGECamera

class SAGERole:
    def __init__(self, mesh, shader, transform, light_pos=None, light_color=None, object_color=None):
        self.mesh = mesh
        self.shader = shader
        self.transform = transform
        self.cameras = []  # Список кортежей (name, camera)
        self.lights = []  # Список кортежей (name, position, color)
        self.light_pos = light_pos if light_pos is not None else glm.vec3(1.2, 1.0, 2.0)
        self.light_color = light_color if light_color is not None else glm.vec3(1.0, 1.0, 1.0)
        self.object_color = object_color if object_color is not None else glm.vec3(1.0, 0.5, 0.31)

        # Добавляем камеру по умолчанию, если список камер пуст
        if not self.cameras:
            default_transform = SAGETransform(
                position=glm.vec3(0.0, 5.0, 10.0),  # Камера смотрит из точки (0, 5, 10)
                rotation=glm.vec3(0.0, 0.0, 0.0),
                scale=glm.vec3(1.0, 1.0, 1.0)
            )
            self.add_camera("default_camera", SAGECamera(default_transform))

    def add_camera(self, name, camera):
        self.cameras.append((name, camera))

    def set_active_camera(self, camera_name):
        for name, camera in self.cameras:
            if name == camera_name:
                self.active_camera = camera
                return
        raise ValueError(f"Камера с именем {camera_name} не найдена")

    def set_active_light(self, light_name):
        for name, pos, color in self.lights:
            if name == light_name:
                self.light_pos = pos
                self.light_color = color
                return
        raise ValueError(f"Источник света с именем {light_name} не найден")

    def draw(self, projection):
        if not hasattr(self, 'active_camera'):
            raise ValueError("Активная камера не установлена")
        view = self.active_camera.get_view_matrix()

        model = self.transform.get_model_matrix()
        self.shader.use()
        self.shader.set_mat4("model", model)
        self.shader.set_mat4("view", view)
        self.shader.set_mat4("projection", projection)
        self.shader.set_vec3("lightPos", self.light_pos)
        self.shader.set_vec3("lightColor", self.light_color)
        self.shader.set_vec3("objectColor", self.object_color)
        self.mesh.draw()