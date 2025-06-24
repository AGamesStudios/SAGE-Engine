ENGINE_VERSION = '2D prototype v0.0.01a'

from .core import GameObject, Scene, Engine, Project, Camera
from .renderers import OpenGLRenderer, Renderer, GLSettings
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
    'clear_image_cache', 'Renderer', 'OpenGLRenderer', 'GLSettings', 'main',
    'ENGINE_VERSION', 'logger', 'ResourceManager', 'set_resource_root', 'get_resource_path',
    'load_project', 'save_project', 'run_project', 'create_engine',
    'load_scene', 'save_scene', 'run_scene',
    'warn', 'error', 'exception'
]
