import json
import pywavefront
import logging
from pyglm import glm
from sage_transform import SAGETransform  # Добавляем импорт в начало файла
from sage_mesh import SAGEMesh
from sage_shader import SAGEShader
from sage_role import SAGERole
from sage_camera import SAGECamera

logging.getLogger('pywavefront').setLevel(logging.ERROR)

class SAGERoleLoader:
    @staticmethod
    def get_attribute_slices(vertex_length):
        if vertex_length == 3:
            return {'position': slice(0, 3)}
        elif vertex_length == 5:
            return {'texcoord': slice(0, 2), 'position': slice(2, 5)}
        elif vertex_length == 6:
            return {'position': slice(0, 3), 'normal': slice(3, 6)}
        elif vertex_length == 8:
            return {'texcoord': slice(0, 2), 'position': slice(2, 5), 'normal': slice(5, 8)}
        else:
            raise ValueError(f"Неподдерживаемая длина вершины: {vertex_length}")

    @staticmethod
    def load_from_json(json_file, platform_transform=None):
        try:
            with open(json_file, 'r') as f:
                data = json.load(f)
            if 'obj_file' not in data:
                raise KeyError("Отсутствует поле 'obj_file' в JSON")

            scene = pywavefront.Wavefront(data['obj_file'], collect_faces=True)
            if not scene.vertices:
                raise ValueError("В OBJ-файле отсутствуют вершины")
            vertex_length = len(scene.vertices[0])
            attr_slices = SAGERoleLoader.get_attribute_slices(vertex_length)

            positions = [coord for vertex in scene.vertices for coord in vertex[attr_slices['position']]]
            tex_coords = [coord for vertex in scene.vertices for coord in vertex.get(attr_slices.get('texcoord', []), [])] if 'texcoord' in attr_slices else None
            normals = [coord for vertex in scene.vertices for coord in vertex.get(attr_slices.get('normal', []), [])] if 'normal' in attr_slices else None
            indices = [idx for mesh in scene.mesh_list for face in mesh.faces for idx in face]

            mesh = SAGEMesh(positions, indices, tex_coords, normals)

            shader = SAGEShader(data['shader']['vertex'], data['shader']['fragment'])

            # Используем трансформ из platform.json, если он передан, иначе из json_file
            transform_data = platform_transform if platform_transform else data['transform']
            transform = SAGETransform(
                position=glm.vec3(*transform_data['position']),
                rotation=glm.vec3(*transform_data['rotation']),
                scale=glm.vec3(*transform_data['scale'])
            )

            if 'lighting' not in data:
                raise KeyError("Отсутствует секция 'lighting' в JSON")
            lighting = data['lighting']
            lights = lighting.get('lights', [])
            if not lights:
                raise ValueError("Список источников света 'lights' пуст или отсутствует")
            light_pos = glm.vec3(*lights[0]['position'])  # Берем первый источник света по умолчанию
            light_color = glm.vec3(*lights[0]['color'])
            object_color = glm.vec3(*lighting.get('objectColor', [1.0, 0.5, 0.31]))

            role = SAGERole(mesh, shader, transform, light_pos, light_color, object_color)
            role.file_path = json_file  # Добавляем путь к файлу роли

            # Загружаем все камеры
            for cam_data in data.get('cameras', []):
                cam_name = cam_data.get('name', f"camera_{len(role.cameras)}")
                cam_transform = SAGETransform(
                    position=glm.vec3(*cam_data['position']),
                    rotation=glm.vec3(*cam_data['rotation']),
                    scale=glm.vec3(*cam_data['scale'])
                )
                role.add_camera(cam_name, SAGECamera(cam_transform))

            # Сохраняем все источники света для возможного использования позже
            role.lights = [(light['name'], glm.vec3(*light['position']), glm.vec3(*light['color']))
                          for light in lights]

            return role
        except (FileNotFoundError, json.JSONDecodeError, KeyError, ValueError) as e:
            raise Exception(f"Ошибка обработки JSON или OBJ: {e}")