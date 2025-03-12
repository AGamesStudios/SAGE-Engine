from pyglm import glm
import json
from sage_transform import SAGETransform
from sage_camera import SAGECamera
from sage_mesh import SAGEMesh
from sage_shader import SAGEShader
from sage_role import SAGERole
from sage_role_loader import SAGERoleLoader
from sage_scene import SAGEScene
from sage_renderer import SAGERenderer
from sage_window import SAGEWindow
from sage_platform import SAGEPlatform
from OpenGL.GL import glEnable, GL_DEPTH_TEST

class SAGEEngine:
    def __init__(self, config):
        """Инициализация движка с использованием платформы."""
        self.window = SAGEWindow(config)
        self.renderer = SAGERenderer()
        self.scene = SAGEScene(self.renderer)
        self.platform = SAGEPlatform(config.get("platform_file", "platform.json"))

        try:
            for role in self.platform.get_roles():
                self.scene.add_role(role)
                # Активируем первую камеру и первый источник света для каждой роли
                if role.cameras:
                    role.set_active_camera(role.cameras[0][0])  # Активируем по имени первой камеры
                if role.lights:
                    role.set_active_light(role.lights[0][0])   # Активируем по имени первого света
        except Exception as e:
            print(f"Ошибка загрузки платформы или ролей: {e}")
            raise

    def run(self):
        """Запуск основного цикла рендеринга."""
        glEnable(GL_DEPTH_TEST)  # Включение теста глубины один раз
        while not self.window.should_close():
            self.window.poll_events()
            for role in self.scene.roles:
                role.transform.rotation.y += 0.01  # Вращение объекта
                role.draw(self.window.get_projection())
            self.window.swap_buffers()

        # Очистка ресурсов
        for role in self.scene.roles:
            role.mesh.cleanup()
            role.shader.cleanup()
        self.window.terminate()

if __name__ == "__main__":
    config = {
        "width": 800,
        "height": 600,
        "title": "SAGE Engine",
        "platform_file": "platform.json"
    }
    try:
        engine = SAGEEngine(config)
        engine.run()
    except Exception as e:
        print(f"Ошибка запуска движка: {e}")