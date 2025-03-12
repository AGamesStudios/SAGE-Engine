class SAGEScene:
    def __init__(self, renderer):
        self.renderer = renderer
        self.roles = []
        self.main_camera = None

    def add_role(self, role):
        self.roles.append(role)

    def set_main_camera(self, camera):
        self.main_camera = camera

    def render(self, projection):
        """Отрисовывает сцену."""
        self.renderer.clear()
        if self.main_camera:
            view = self.main_camera.get_view_matrix()
            for role in self.roles:
                role.draw(view, projection)