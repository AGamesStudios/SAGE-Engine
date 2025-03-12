import glm

class SAGERole:
    def __init__(self, mesh, shader, transform, object_color, script=None):
        self.mesh = mesh              # Встроенный меш
        self.shader = shader          # Встроенный шейдер
        self.transform = transform    # Встроенная трансформация
        self.object_color = object_color  # Встроенный цвет
        self.script = script          # Привязанный Python-скрипт (функция)

    def update(self, delta_time):
        """Вызывается на каждом кадре для обновления логики роли"""
        if self.script:
            self.script(self, delta_time)  # Выполнение скрипта, если он есть

    def draw(self, view, projection, light_pos, light_color):
        """Отрисовка роли"""
        model = self.transform.get_model_matrix()
        self.shader.use()
        self.shader.set_mat4("model", model)
        self.shader.set_mat4("view", view)
        self.shader.set_mat4("projection", projection)
        self.shader.set_vec3("lightPos", light_pos)
        self.shader.set_vec3("lightColor", light_color)
        self.shader.set_vec3("objectColor", self.object_color)
        self.mesh.draw()