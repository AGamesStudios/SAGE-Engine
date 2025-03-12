import json
import os
from pyglm import glm
from sage_role_loader import SAGERoleLoader
from sage_transform import SAGETransform
from sage_camera import SAGECamera
from sage_role import SAGERole


class SAGEPlatform:
    def __init__(self, platform_file="platform.json"):
        """Инициализация платформы с указанием файла конфигурации."""
        self.platform_file = platform_file
        self.roles = []
        self.load_platform()

    def load_platform(self):
        """Загружает конфигурацию платформы из JSON-файла."""
        try:
            if not os.path.exists(self.platform_file):
                raise FileNotFoundError(f"Файл платформы {self.platform_file} не найден")

            with open(self.platform_file, 'r') as f:
                data = json.load(f)

            self.load_roles(data.get("roles", []))
        except (json.JSONDecodeError, FileNotFoundError) as e:
            raise Exception(f"Ошибка загрузки платформы: {e}")

    def load_roles(self, role_configs):
        """Загружает роли из списка конфигураций."""
        self.roles.clear()
        for role_config in role_configs:
            role_file = role_config.get("file")
            transform = role_config.get("transform")
            if os.path.exists(role_file):
                try:
                    role = SAGERoleLoader.load_from_json(role_file, transform)
                    self.roles.append(role)
                except Exception as e:
                    print(f"Ошибка загрузки роли из {role_file}: {e}")
            else:
                print(f"Файл роли {role_file} не найден, пропущен")

    def add_role(self, role_file, transform=None):
        """Добавляет новую роль из указанного JSON-файла с трансформом."""
        if os.path.exists(role_file):
            try:
                role = SAGERoleLoader.load_from_json(role_file, transform)
                self.roles.append(role)
                self.save_platform()  # Сохраняем обновленный список ролей
            except Exception as e:
                print(f"Ошибка добавления роли из {role_file}: {e}")
        else:
            print(f"Файл роли {role_file} не найден")

    def remove_role(self, role_file):
        """Удаляет роль по указанному пути файла."""
        self.roles = [role for role in self.roles if role.file_path != role_file]
        self.save_platform()  # Сохраняем обновленный список ролей

    def save_platform(self):
        """Сохраняет текущую конфигурацию платформы в JSON-файл."""
        data = {
            "roles": [{"file": role.file_path, "transform": {
                "position": [role.transform.position.x, role.transform.position.y, role.transform.position.z],
                "rotation": [role.transform.rotation.x, role.transform.rotation.y, role.transform.rotation.z],
                "scale": [role.transform.scale.x, role.transform.scale.y, role.transform.scale.z]
            }} for role in self.roles if hasattr(role, 'file_path')]
        }
        with open(self.platform_file, 'w') as f:
            json.dump(data, f, indent=4)

    def get_roles(self):
        """Возвращает список загруженных ролей."""
        return self.roles

    def get_default_camera(self):
        """Создает камеру по умолчанию (можно убрать, если не нужно)."""
        # Этот метод можно удалить, если камеры задаются индивидуально в ролях
        position = glm.vec3(0.0, 20.0, 1.0)
        rotation = glm.vec3(0.0, 0.0, 0.0)
        scale = glm.vec3(1.0, 1.0, 1.0)
        transform = SAGETransform(position, rotation, scale)
        return SAGECamera(transform)