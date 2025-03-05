import logging
from ursina import *


# Centralized logging setup
logging.basicConfig(level=logging.INFO, format='[%(levelname)s] %(message)s')
logger = logging.getLogger('SAGEEngine')


# ==================== SAGERole ====================
class SAGERole(Entity):
    """
    The base class for all game objects (roles).
    Easily extendable via inheritance.
    """
    def __init__(self, role_name: str, **kwargs: Any):
        super().__init__()
        self.role = role_name
        self.position = kwargs.get('position', Vec3(0, 0, 0))
        self.rotation = kwargs.get('rotation', Vec3(0, 0, 0))
        self.scale = kwargs.get('scale', Vec3(1, 1, 1))
        self.model = kwargs.get('model', 'cube')
        self.texture = kwargs.get('texture', 'white_cube')
        self.collider = kwargs.get('collider', 'box')
        self.visible = kwargs.get('visible', True)

        self._setup_role()

    def _setup_role(self):
        """Sets up the role depending on its type."""
        if self.role == 'player':
            self.model = 'models/SAGE Bot.obj'
            self.collider = 'box'
            self.texture = 'white_cube'
        elif self.role == 'map':
            self.model = 'plane'
            self.scale = Vec3(50, 1, 50)
            self.texture = 'grass'
            self.collider = 'mesh'
        elif self.role == 'camera':
            camera.position = self.position
            camera.rotation = self.rotation
            self.visible = False

    def update(self):
        """Updates the role's state."""
        pass  # Add custom logic for updates, physics, etc. (can be overridden)


# ==================== SAGEEngine ====================
class SAGEEngine(Ursina):
    """
    Main game engine based on Ursina.
    Version: 0.3 (optimized and easily extendable)
    """
    def __init__(self, width: int = 1920, height: int = 1080, title: str = "SAGE Engine"):
        super().__init__()

        self.window_title = title
        window.size = (width, height)
        self.gravity = 0.5

        self.roles = []
        self.script_manager = SAGEScriptManager(self)

        # Setup default scene
        self.setup_default_scene()

    def setup_default_scene(self):
        """Creates a default scene if no roles were loaded."""
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

    def update(self):
        """Main engine update loop."""
        for role in self.roles:
            role.update()

    def add_role(self, role: SAGERole):
        """Adds a new role to the engine."""
        self.roles.append(role)

    def run(self):
        """Runs the engine."""
        logger.info("Running the engine...")
        self.app.run()


# ==================== SAGEScriptManager ====================
class SAGEScriptManager:
    """
    Manages parsing and execution of commands from scripts (.ssc).
    The API for commands remains unchanged.
    Supports dynamically adding new commands.
    """
    def __init__(self, engine: 'SAGEEngine'):
        self.engine = engine
        self.commands = {
            'create_role': self.create_role,
            'set_map': self.set_map,
            'bind_input': self.bind_input,
            'set_window_position': self.set_window_position,
            'set_gravity': self.set_gravity,
        }

    def execute(self, command_name: str, args: Tuple[Any, ...], kwargs: Dict[str, Any]):
        """Executes a command from the script."""
        if command_name in self.commands:
            try:
                self.commands[command_name](*args, **kwargs)
            except Exception as e:
                logger.error(f"Error in command '{command_name}': {e}")
        else:
            logger.warning(f"Unknown command: '{command_name}'")

    def create_role(self, role_name: str, **kwargs):
        """Creates a new role with parameters."""
        logger.info(f"Creating role: {role_name} with parameters: {kwargs}")
        try:
            role = SAGERole(role_name, **kwargs)
            self.engine.add_role(role)
        except Exception as e:
            logger.error(f"Error creating role '{role_name}': {e}")

    def set_map(self, **kwargs):
        """Sets up the map as a role."""
        logger.info(f"Setting up map with parameters: {kwargs}")
        try:
            map_role = SAGERole('map', **kwargs)
            self.engine.add_role(map_role)
        except Exception as e:
            logger.error(f"Error setting up map: {e}")

    def bind_input(self, key: str, role_name: str, action: str, value: Any):
        """Binds input to a role's action."""
        role = next((role for role in self.engine.roles if role.role == role_name), None)
        if not role:
            logger.error(f"Role '{role_name}' not found")
            return
        if action not in ('move', 'rotate', 'scale', 'jump'):
            logger.error(f"Invalid action '{action}'")
            return
        vector = Vec3(*value) if action != 'jump' else value

        def perform_action():
            if action == 'move':
                role.position += vector
            elif action == 'rotate':
                role.rotation += vector
            elif action == 'scale':
                role.scale += vector
            elif action == 'jump' and role.on_ground:
                role.velocity_y = vector
                role.on_ground = False

        self.engine.input.bind(key, perform_action)
        logger.info(f"Bound '{key}' to {action} for {role_name} with {value}")

    def set_window_position(self, position: Tuple[int, int]):
        """Sets the window position."""
        window.position = Vec2(*position)
        logger.info(f"Window position set to: {position}")

    def set_gravity(self, value: float):
        """Sets gravity strength."""
        self.engine.gravity = value
        logger.info(f"Gravity set to: {value}")


# ==================== Engine Execution ====================
if __name__ == '__main__':
    engine = SAGEEngine()
    engine.run()
