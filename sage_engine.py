import os
import re
import ast
import logging
from typing import Any, Callable, Dict, List, Tuple, Optional
from ursina import *

# Настройка централизованного логирования
logging.basicConfig(level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger('SAGEEngine')

# ==================== SAGEScriptManager ====================
class SAGEScriptManager:
    """
    Управляет парсингом и выполнением команд из скрипта (.ssc).
    API команд не изменяется.
    """
    def __init__(self, engine: 'SAGEEngine'):
        self.engine = engine
        self.commands: Dict[str, Callable[..., None]] = {
            'create_role': self.create_role,
            'set_map': self.set_map,
            'bind_input': self.bind_input,
            'set_window_position': self.set_window_position,
            'set_gravity': self.set_gravity,
        }

    def parse_args(self, arg_str: str) -> Tuple[Tuple[Any, ...], Dict[str, Any]]:
        """Разбирает аргументы из строки команды."""
        if not arg_str:
            return (), {}
        try:
            tree = ast.parse(f"dummy({arg_str})")
            call = tree.body[0].value
            args = tuple(ast.literal_eval(arg) for arg in call.args)
            kwargs = {kw.arg: ast.literal_eval(kw.value) for kw in call.keywords}
            return args, kwargs
        except Exception as e:
            logger.error(f"Ошибка разбора аргументов: {e}")
            return (), {}

    def execute(self, command_name: str, args: Tuple[Any, ...], kwargs: Dict[str, Any]) -> None:
        """Выполняет команду из скрипта."""
        if command_name in self.commands:
            try:
                self.commands[command_name](*args, **kwargs)
            except Exception as e:
                logger.error(f"Ошибка в команде '{command_name}': {e}")
        else:
            logger.warning(f"Неизвестная команда: '{command_name}'")

    def create_role(self, role_name: str, **kwargs: Any) -> None:
        """Создаёт новую роль с параметрами."""
        logger.info(f"Создание роли: {role_name} с параметрами: {kwargs}")
        try:
            role = SAGERole(role_name, **kwargs)
            self.engine.add_role(role)
        except Exception as e:
            logger.error(f"Ошибка создания роли '{role_name}': {e}")

    def set_map(self, **kwargs: Any) -> None:
        """Настраивает карту как роль."""
        logger.info(f"Установка карты с параметрами: {kwargs}")
        try:
            map_role = SAGERole('map', **kwargs)
            self.engine.add_role(map_role)
        except Exception as e:
            logger.error(f"Ошибка установки карты: {e}")

    def bind_input(self, key: str, role_name: str, action: str, value: Any) -> None:
        """Привязывает ввод к действию роли."""
        role = self.engine.get_role(role_name)
        if not role:
            logger.error(f"Роль '{role_name}' не найдена")
            return
        if action not in ('move', 'rotate', 'scale', 'jump'):
            logger.error(f"Неверное действие '{action}'")
            return
        vector = Vec3(*value) if action != 'jump' else value

        def perform_action() -> None:
            try:
                if action == 'move':
                    role.position += vector
                elif action == 'rotate':
                    role.rotation += vector
                elif action == 'scale':
                    role.scale += vector
                elif action == 'jump' and role.on_ground:
                    role.velocity_y = vector  # Скорость прыжка
                    role.on_ground = False
            except Exception as e:
                logger.error(f"Ошибка выполнения действия '{action}': {e}")

        self.engine.bind_input(key, perform_action)
        logger.info(f"Привязка '{key}' к {action} для {role_name} с {value}")

    def set_window_position(self, position: Tuple[int, int]) -> None:
        """Устанавливает позицию окна."""
        window.position = Vec2(*position)
        logger.info(f"Установлена позиция окна: {position}")

    def set_gravity(self, value: float) -> None:
        """Устанавливает силу гравитации."""
        self.engine.gravity = value
        logger.info(f"Установлена гравитация: {value}")

# ==================== SAGEInput ====================
class SAGEInput:
    """
    Обрабатывает ввод с клавиатуры.
    Реализует метод update(), который каждую итерацию цикла проверяет состояние нажатых клавиш (held_keys)
    и вызывает привязанные действия, что обеспечивает непрерывное реагирование.
    """
    def __init__(self):
        self.handlers: Dict[str, List[Callable[[], None]]] = {}

    def bind(self, key: str, action: Callable[[], None]) -> None:
        self.handlers.setdefault(key.lower(), []).append(action)

    def process(self, key: str) -> None:
        """
        Вызывается для одноразовой обработки события ввода.
        """
        for action in self.handlers.get(key.lower(), []):
            try:
                action()
            except Exception as e:
                logger.error(f"Ошибка обработки ввода '{key}': {e}")

    def update(self) -> None:
        """
        Каждую итерацию цикла проверяет состояние нажатых клавиш из held_keys и выполняет привязанные действия.
        Это позволяет реагировать на удержание клавиш.
        """
        from ursina import held_keys
        for key, actions in self.handlers.items():
            if held_keys.get(key, False):
                for action in actions:
                    try:
                        action()
                    except Exception as e:
                        logger.error(f"Ошибка выполнения действия для удерживаемой клавиши '{key}': {e}")

# ==================== SAGERole ====================
class SAGERole(Entity):
    """
    Базовый класс для всех игровых объектов (ролей).
    Легко расширяемый за счёт наследования.
    """
    def __init__(self, role_name: str, **kwargs: Any):
        super().__init__()
        self.role: str = role_name
        self.position: Vec3 = Vec3(*kwargs.get('position', (0, 0, 0)))
        self.rotation: Vec3 = Vec3(*kwargs.get('rotation', (0, 0, 0)))
        self.scale: Vec3 = Vec3(*kwargs.get('scale', (1, 1, 1)))
        self.model: Any = kwargs.get('model', None)
        self.texture: Optional[str] = kwargs.get('texture', None)
        self.collider: Optional[Any] = kwargs.get('collider', None)
        self.visible: bool = kwargs.get('visible', True)
        # Физика
        self.velocity_y: float = 0.0
        self.on_ground: bool = False
        # Ссылка на движок (назначается в update)
        self.scene: Optional['SAGEEngine'] = None

        self._setup_role(kwargs)

    def _setup_role(self, kwargs: Dict[str, Any]) -> None:
        """
        Настраивает роль в зависимости от её типа.
        Для расширения достаточно создать наследника с переопределением данного метода.
        """
        try:
            if self.role == 'player':
                self.model = self.model or 'models/SAGE Bot.obj'
                if isinstance(self.model, str) and not os.path.exists(self.model):
                    logger.warning(f"Модель '{self.model}' не найдена, используем куб")
                    self.model = 'cube'
                self.collider = self.collider or 'box'
                self.texture = self.texture or 'white_cube'
            elif self.role == 'map':
                self.model = self.model or 'plane'
                self.scale = Vec3(*kwargs.get('scale', (50, 1, 50)))
                self.texture = kwargs.get('texture', 'grass')
                self.collider = self.collider or 'mesh'
            elif self.role == 'light':
                light_type = kwargs.get('light_type', 'directional')
                if light_type == 'directional':
                    light = DirectionalLight(shadows=True)
                    light.set_position(self.position)
                    Entity(light=light)
                self.model = None
                self.visible = False
            elif self.role == 'camera':
                self.model = None
                self.visible = False
                camera.position = self.position
                camera.rotation = self.rotation
        except Exception as e:
            logger.error(f"Ошибка при настройке роли '{self.role}': {e}")

    def update(self) -> None:
        """
        Обновляет состояние роли с учётом физики.
        Для расширения можно переопределить метод update в наследниках.
        """
        if self.role == 'player':
            gravity = self.scene.gravity if self.scene and hasattr(self.scene, 'gravity') else 0.5
            self.velocity_y -= gravity * time.dt * 20
            self.y += self.velocity_y * time.dt
            if self.y <= 0:
                self.y = 0
                self.velocity_y = 0
                self.on_ground = True
        elif self.role == 'camera':
            camera.position = self.position
            camera.rotation = self.rotation

# ==================== SAGEEngine ====================
class SAGEEngine:
    """
    Основной движок, управляющий ролями и скриптами.
    Версия: 0.3 (оптимизирован и легко расширяемый)
    """
    def __init__(self, width: int = 1920, height: int = 1080, title: str = "SAGE Engine v0.3"):
        self.version: str = "0.3"
        self.app = Ursina()
        window.title = title
        window.size = (width, height)
        window.borderless = False
        window.fullscreen = False
        window.exit_button.visible = True
        window.fps_counter.enabled = True

        # Хранение ролей
        self.roles: List[SAGERole] = []
        self.script_manager = SAGEScriptManager(self)
        self.input = SAGEInput()
        self.gravity: float = 0.5

        self.load_scripts()
        self.setup_default_scene()

        # Используем наш обработчик ввода и дополняем update обработкой held_keys
        self.app.input = self.handle_input
        self.app.update = self.update

    def load_scripts(self, filename: str = "scripts/game_logic.ssc") -> None:
        """
        Загружает скрипт из файла .ssc.
        Если файла нет, создаётся шаблонный скрипт.
        """
        if not os.path.exists(filename):
            logger.warning(f"Файл '{filename}' не найден, создаём стандартный скрипт.")
            os.makedirs(os.path.dirname(filename), exist_ok=True)
            with open(filename, 'w', encoding='utf-8') as f:
                f.write("# SAGE Engine v0.3 Стандартный скрипт\n")
                f.write("set_window_position((192, 108))\n")
                f.write("set_gravity(0.5)\n")
                f.write("set_map(scale=(50, 1, 50), texture='grass')\n")
                f.write("create_role('player', position=(0, 1, 0), model='models/SAGE Bot.obj', collider='box')\n")
                f.write("create_role('light', position=(5, 5, 5), light_type='directional')\n")
                f.write("create_role('camera', position=(0, 10, -10))\n")
                f.write("bind_input('w', 'player', 'move', (0, 0, -0.1))\n")
                f.write("bind_input('s', 'player', 'move', (0, 0, 0.1))\n")
                f.write("bind_input('a', 'player', 'move', (-0.1, 0, 0))\n")
                f.write("bind_input('d', 'player', 'move', (0.1, 0, 0))\n")
                f.write("bind_input('space', 'player', 'jump', 5)\n")
        with open(filename, "r", encoding="utf-8") as file:
            for line in file:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                match = re.match(r"(\w+)\((.*)\)", line)
                if match:
                    command_name, arg_str = match.groups()
                    args, kwargs = self.script_manager.parse_args(arg_str)
                    self.script_manager.execute(command_name, args, kwargs)

    def setup_default_scene(self) -> None:
        """
        Создаёт стандартную сцену, если роли не были загружены.
        """
        if not self.roles:
            self.script_manager.set_window_position((192, 108))
            self.script_manager.set_gravity(0.5)
            self.script_manager.set_map(scale=(50, 1, 50), texture='grass')
            self.script_manager.create_role('player', position=(0, 1, 0), model='models/SAGE Bot.obj', collider='box')
            self.script_manager.create_role('light', position=(5, 5, 5), light_type='directional')
            self.script_manager.create_role('camera', position=(0, 10, -10))
            self.script_manager.bind_input('w', 'player', 'move', (0, 0, -0.1))
            self.script_manager.bind_input('s', 'player', 'move', (0, 0, 0.1))
            self.script_manager.bind_input('a', 'player', 'move', (-0.1, 0, 0))
            self.script_manager.bind_input('d', 'player', 'move', (0.1, 0, 0))
            self.script_manager.bind_input('space', 'player', 'jump', 5)

    def handle_input(self, key: str) -> None:
        """
        Вызывается при событии ввода (одноразовая обработка).
        """
        self.input.process(key)

    def update(self) -> None:
        """
        Основной цикл обновления движка.
        Каждая роль получает ссылку на движок, затем вызывается их метод update.
        Также вызывается update системы ввода для обработки удерживаемых клавиш.
        """
        self.input.update()
        for role in self.roles:
            role.scene = self
            role.update()

    def add_role(self, role: SAGERole) -> None:
        """Добавляет новую роль в движок."""
        self.roles.append(role)

    def run(self) -> None:
        """Запускает игровой движок."""
        logger.info("Запуск движка...")
        self.app.run()

    def get_role(self, role_name: str) -> Optional[SAGERole]:
        """Находит роль по имени."""
        return next((role for role in self.roles if role.role == role_name), None)

    def bind_input(self, key: str, action: Callable[[], None]) -> None:
        """Привязывает ввод к действию."""
        self.input.bind(key, action)

# ==================== Запуск движка ====================
if __name__ == '__main__':
    engine = SAGEEngine()
    engine.run()
