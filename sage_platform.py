import json
import glm
from sage_transform import SAGETransform
from sage_camera import SAGECamera
from sage_light import SAGELight
from sage_mesh import SAGEMesh
from sage_shader import SAGEShader
from sage_role import SAGERole
import importlib.util

class SAGEPlatform:
    def __init__(self, platform_file="platform.json"):
        self.roles = []
        self.cameras = []
        self.lights = []
        self.active_camera = None
        self.load_platform(platform_file)

    def load_platform(self, platform_file):
        with open(platform_file, 'r') as f:
            data = json.load(f)

        # Загрузка камер
        for cam_data in data['cameras']:
            transform = SAGETransform(
                position=glm.vec3(*cam_data['position']),
                rotation=glm.vec3(*cam_data['rotation']),
                scale=glm.vec3(*cam_data['scale'])
            )
            camera = SAGECamera(transform)
            self.cameras.append(camera)
            if cam_data.get('active', False):
                self.active_camera = camera

        # Загрузка источников света
        for light_data in data['lights']:
            light = SAGELight(
                position=glm.vec3(*light_data['position']),
                color=glm.vec3(*light_data['color'])
            )
            self.lights.append(light)

        # Загрузка ролей
        for role_config in data['roles']:
            role_file = role_config['file']
            transform_data = role_config['transform']
            transform = SAGETransform(
                position=glm.vec3(*transform_data['position']),
                rotation=glm.vec3(*transform_data['rotation']),
                scale=glm.vec3(*transform_data['scale'])
            )
            with open(role_file, 'r') as f:
                role_data = json.load(f)
            mesh = SAGEMesh(role_data['obj_file'])
            shader = SAGEShader(role_data['shader']['vertex'], role_data['shader']['fragment'])
            object_color = glm.vec3(*role_data['objectColor'])

            # Загрузка скрипта, если он указан
            script = None
            if 'script' in role_config:
                script_path = role_config['script']
                spec = importlib.util.spec_from_file_location("script_module", script_path)
                script_module = importlib.util.module_from_spec(spec)
                spec.loader.exec_module(script_module)
                script = script_module.update  # Ожидаем функцию update в скрипте

            role = SAGERole(mesh, shader, transform, object_color, script)
            self.roles.append(role)

    def update(self, delta_time):
        """Обновление всех ролей"""
        for role in self.roles:
            role.update(delta_time)

    def render(self, projection):
        """Отрисовка сцены"""
        if not self.active_camera:
            raise ValueError("Активная камера не установлена")
        view = self.active_camera.get_view_matrix()
        for role in self.roles:
            for light in self.lights:
                role.draw(view, projection, light.position, light.color)