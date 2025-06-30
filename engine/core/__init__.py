from .game_object import GameObject, clear_image_cache
from .scene import Scene
from .project import Project
from .scene_graph import SceneGraph
from .camera import Camera
from .objects import (
    register_object,
    object_from_dict,
    object_to_dict,
)
from .effects import register_effect, get_effect, EFFECT_REGISTRY
from .post_effects import (
    register_post_effect,
    get_post_effect,
    POST_EFFECT_REGISTRY,
)

__all__ = [
    'GameObject',
    'Scene',
    'SceneGraph',
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
    'register_post_effect',
    'get_post_effect',
    'POST_EFFECT_REGISTRY',
]


def __getattr__(name):
    if name == 'Engine':
        from .engine import Engine
        return Engine
    if name == 'EngineSettings':
        from .settings import EngineSettings
        return EngineSettings
    raise AttributeError(name)
