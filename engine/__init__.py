ENGINE_VERSION = '2D prototype v0.0.01a'

from .core import GameObject, Scene, Engine, Project, Camera
from .renderers import (
    OpenGLRenderer,
    QtPainterRenderer,
    Renderer,
)
from .game_window import GameWindow
from .core.game_object import clear_image_cache
from .api import (
    load_project,
    save_project,
    run_project,
    create_engine,
    load_scene,
    save_scene,
    run_scene,
)
from .core.resources import ResourceManager, set_resource_root, get_resource_path
from .core.engine import main
from .log import logger
from .diagnostics import warn, error, exception
from . import units
set_units_per_meter = units.set_units_per_meter
meters = units.meters
kilometers = units.kilometers
to_units = units.to_units
from_units = units.from_units
set_y_up = units.set_y_up
Y_UP = units.Y_UP
from sage_sdk.plugins import register_plugin as register_engine_plugin, load_plugins as _load_engine_plugins

def load_engine_plugins(engine, paths=None):
    """Load plugins targeting the engine."""
    _load_engine_plugins('engine', engine, paths)
from .logic.base import Event, EventSystem
from .logic.conditions import (
    KeyPressed, KeyReleased, MouseButton, InputState, Collision,
    AfterTime, OnStart, EveryFrame, VariableCompare, ZoomAbove,
)
from .logic.actions import (
    Move, SetPosition, Destroy, Print, PlaySound, Spawn,
    SetVariable, ModifyVariable, SetZoom,
)

__all__ = [
    'GameObject', 'Scene', 'Engine', 'EventSystem',
    'Event', 'KeyPressed', 'KeyReleased', 'MouseButton', 'InputState', 'Collision', 'AfterTime',
    'Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn',
    'OnStart', 'EveryFrame', 'VariableCompare', 'ZoomAbove',
    'SetVariable', 'ModifyVariable', 'SetZoom', 'Project', 'Camera',
    'clear_image_cache', 'Renderer', 'OpenGLRenderer', 'QtPainterRenderer', 'main',
    'ENGINE_VERSION', 'logger', 'ResourceManager', 'set_resource_root', 'get_resource_path',
    'load_project', 'save_project', 'run_project', 'create_engine',
    'load_scene', 'save_scene', 'run_scene',
    'warn', 'error', 'exception',
    'register_engine_plugin', 'load_engine_plugins',
    'units', 'set_units_per_meter', 'meters', 'kilometers', 'to_units', 'from_units',
    'set_y_up', 'Y_UP', 'GameWindow'
]

# validate that all exported names exist and warn if any are missing
_missing = [name for name in __all__ if name not in globals()]
for _name in _missing:
    warn("Missing reference %s in __init__", _name)
del _missing
