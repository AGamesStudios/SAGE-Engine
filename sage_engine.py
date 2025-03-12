from sage_window import SAGEWindow
from sage_platform import SAGEPlatform
from OpenGL.GL import *
import time


class SAGEEngine:
    def __init__(self, config):
        self.window = SAGEWindow(config['width'], config['height'], config['title'])
        self.platform = SAGEPlatform(config.get("platform_file", "platform.json"))
        self.last_time = time.time()

    def run(self):
        while not self.window.should_close():
            current_time = time.time()
            delta_time = current_time - self.last_time
            self.last_time = current_time

            self.window.poll_events()
            self.platform.update(delta_time)  # Вызов обновления скриптов
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
            self.platform.render(self.window.get_projection())
            self.window.swap_buffers()
        self.window.terminate()


if __name__ == "__main__":
    # Конфигурация движка
    config = {
        "width": 800,  # Ширина окна
        "height": 600,  # Высота окна
        "title": "SAGE Engine",  # Заголовок окна
        "platform_file": "platform.json"  # Файл конфигурации платформы
    }

    try:
        # Создание и запуск движка
        engine = SAGEEngine(config)
        engine.run()
    except Exception as e:
        print(f"Ошибка запуска движка: {e}")