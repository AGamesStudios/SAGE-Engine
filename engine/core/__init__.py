from .game_object import GameObject, clear_image_cache
from .scene import Scene
from .project import Project
from .camera import Camera
from .objects import (
    register_object,
    object_from_dict,
    object_to_dict,
)
from .effects import register_effect, get_effect, EFFECT_REGISTRY

__all__ = [
    'GameObject',
    'Scene',
    'Project',
    'Camera',
    'clear_image_cache',
    'register_object',
    'object_from_dict',
    'object_to_dict',
    'Engine',
    'EngineSettings',
    'register_effect',
    'get_effect',
    'EFFECT_REGISTRY',
]


def __getattr__(name):
    if name == 'Engine':
        from .engine import Engine
        return Engine
    if name == 'EngineSettings':
        from .settings import EngineSettings
        return EngineSettings
    raise AttributeError(name)
