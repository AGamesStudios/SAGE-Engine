"""Top level package for the SAGE engine with lazy imports."""

from importlib import import_module
from typing import TYPE_CHECKING

from .version import __version__, require as require_version
from .utils.log import logger
from .utils.diagnostics import warn, error, exception  # noqa: F401

ENGINE_VERSION = __version__
try:
    from sage_sdk.plugins import PluginManager  # noqa: E402
except Exception:  # pragma: no cover - sdk optional
    class PluginManager:  # type: ignore[override]
        """Fallback plugin manager when ``sage_sdk`` is missing."""

        def __init__(self, *a, **k):
            pass

        def register(self, func):
            pass

        def load(self, obj, paths=None):  # noqa: D401
            """No-op plugin loader."""
            logger.warning(
                "Plugins ignored because sage_sdk is unavailable"
            )
            return

ENGINE_PLUGINS = PluginManager('engine')


def register_engine_plugin(func):
    """Register an engine plugin programmatically."""
    ENGINE_PLUGINS.register(func)


def load_engine_plugins(engine, paths=None):
    """Load plugins targeting the engine."""
    ENGINE_PLUGINS.load(engine, paths)


_lazy = {
    'GameObject': ('engine.entities.game_object', 'GameObject'),
    'TileMap': ('engine.entities.tile_map', 'TileMap'),
    'Scene': ('engine.core.scenes.scene', 'Scene'),
    'Project': ('engine.core.project', 'Project'),
    'Camera': ('engine.core.camera', 'Camera'),
    'EngineSettings': ('engine.core.settings', 'EngineSettings'),
    'register_effect': ('engine.core.effects', 'register_effect'),
    'get_effect': ('engine.core.effects', 'get_effect'),
    'EFFECT_REGISTRY': ('engine.core.effects', 'EFFECT_REGISTRY'),
    'register_post_effect': ('engine.core.post_effects', 'register_post_effect'),
    'get_post_effect': ('engine.core.post_effects', 'get_post_effect'),
    'POST_EFFECT_REGISTRY': ('engine.core.post_effects', 'POST_EFFECT_REGISTRY'),
    'Mesh': ('engine.mesh_utils', 'Mesh'),
    'create_square_mesh': ('engine.mesh_utils', 'create_square_mesh'),
    'create_triangle_mesh': ('engine.mesh_utils', 'create_triangle_mesh'),
    'create_circle_mesh': ('engine.mesh_utils', 'create_circle_mesh'),
    'clear_image_cache': ('engine.entities.game_object', 'clear_image_cache'),
    '__version__': ('engine.version', '__version__'),
    'register_object': ('engine.core.objects', 'register_object'),
    'object_from_dict': ('engine.core.objects', 'object_from_dict'),
    'object_to_dict': ('engine.core.objects', 'object_to_dict'),
    'load_project': ('engine.api', 'load_project'),
    'save_project': ('engine.api', 'save_project'),
    'run_project': ('engine.api', 'run_project'),
    'create_engine': ('engine.api', 'create_engine'),
    'load_scene': ('engine.api', 'load_scene'),
    'save_scene': ('engine.api', 'save_scene'),
    'run_scene': ('engine.api', 'run_scene'),
    'SceneFile': ('engine.core.scene_file', 'SceneFile'),
    'load_scene_file': ('engine.core.scene_file', 'load_scene_file'),
    'save_scene_file': ('engine.core.scene_file', 'save_scene_file'),
    'ResourceManager': ('engine.core.resources', 'ResourceManager'),
    'set_resource_root': ('engine.core.resources', 'set_resource_root'),
    'get_resource_path': ('engine.core.resources', 'get_resource_path'),
    'main': ('engine.core.engine', 'main'),
    'Event': ('engine.logic.base', 'Event'),
    'EventSystem': ('engine.logic.base', 'EventSystem'),
    'register_condition': ('engine.logic.base', 'register_condition'),
    'register_action': ('engine.logic.base', 'register_action'),
    'get_registered_conditions': ('engine.logic.base', 'get_registered_conditions'),
    'get_registered_actions': ('engine.logic.base', 'get_registered_actions'),
    'condition_from_dict': ('engine.logic.base', 'condition_from_dict'),
    'action_from_dict': ('engine.logic.base', 'action_from_dict'),
    'event_from_dict': ('engine.logic.base', 'event_from_dict'),
    'units': ('engine.utils.units', None),
    'set_units_per_meter': ('engine.utils.units', 'set_units_per_meter'),
    'meters': ('engine.utils.units', 'meters'),
    'kilometers': ('engine.utils.units', 'kilometers'),
    'to_units': ('engine.utils.units', 'to_units'),
    'from_units': ('engine.utils.units', 'from_units'),
    'set_y_up': ('engine.utils.units', 'set_y_up'),
    'Y_UP': ('engine.utils.units', 'Y_UP'),
    'GameWindow': ('engine.game_window', 'GameWindow'),
    'Engine': ('engine.core.engine', 'Engine'),
    'Renderer': ('engine.renderers', 'Renderer'),
    'OpenGLRenderer': ('engine.renderers.opengl_renderer', 'OpenGLRenderer'),
    'NullRenderer': ('engine.renderers.null_renderer', 'NullRenderer'),
    'math2d': ('engine.core.math2d', None),
    'SceneGraph': ('engine.core.scene_graph', 'SceneGraph'),
    'SceneNode': ('engine.core.scene_graph', 'SceneNode'),
    'BaseGraph': ('engine.core.scene_graph', 'BaseGraph'),
    'BaseNode': ('engine.core.scene_graph', 'BaseNode'),
    'Object': ('engine.entities.object', 'Object'),
    'Transform2D': ('engine.entities.object', 'Transform2D'),
    'Material': ('engine.entities.object', 'Material'),
    'create_role': ('engine.entities.object', 'create_role'),
    'SceneManager': ('engine.core.scenes.manager', 'SceneManager'),
    'EngineExtension': ('engine.core.extensions', 'EngineExtension'),
    'InputManager': ('engine.inputs', 'InputManager'),
    'NullInput': ('engine.inputs', 'NullInput'),
    'LibraryLoader': ('engine.core.library', 'LibraryLoader'),
    'DEFAULT_LIBRARY_LOADER': ('engine.core.library', 'DEFAULT_LIBRARY_LOADER'),
    'load_engine_libraries': ('engine.core.library', 'load_engine_libraries'),
    'register_draw_handler': ('engine.renderers', 'register_draw_handler'),
    'Cache': ('engine.cache', 'Cache'),
    'SAGE_CACHE': ('engine.cache', 'SAGE_CACHE'),
    'AudioManager': ('engine.audio', 'AudioManager'),
    'load_sageaudio': ('engine.formats', 'load_sageaudio'),
    'save_sageaudio': ('engine.formats', 'save_sageaudio'),
    'load_sagemesh': ('engine.formats', 'load_sagemesh'),
    'save_sagemesh': ('engine.formats', 'save_sagemesh'),
    'Animation': ('engine.animation', 'Animation'),
    'Frame': ('engine.animation', 'Frame'),
    'load_sageanimation': ('engine.formats', 'load_sageanimation'),
    'save_sageanimation': ('engine.formats', 'save_sageanimation'),
    'load_sagelogic': ('engine.formats', 'load_sagelogic'),
    'save_sagelogic': ('engine.formats', 'save_sagelogic'),
    'load_resource': ('engine.formats', 'load_resource'),
    'save_resource': ('engine.formats', 'save_resource'),
    'TextureAtlas': ('engine.texture_atlas', 'TextureAtlas'),
    'GamepadInput': ('engine.inputs.gamepad', 'GamepadInput'),
    'save_game': ('engine.savegame', 'save_game'),
    'load_game': ('engine.savegame', 'load_game'),
}

__all__ = sorted(
    list(_lazy.keys())
    + [
        '__version__', 'ENGINE_VERSION', 'logger',
        'warn', 'error', 'exception',
        'register_engine_plugin', 'load_engine_plugins',
        'require_version',
    ]
)


def __getattr__(name):
    if name in _lazy:
        module_name, attr = _lazy[name]
        module = import_module(module_name)
        value = getattr(module, attr) if attr else module
        globals()[name] = value
        return value
    raise AttributeError(name)


def __dir__():
    return sorted(list(globals().keys()) + list(_lazy.keys()))

if TYPE_CHECKING:  # pragma: no cover - hints for static analyzers
    from .entities.game_object import GameObject, clear_image_cache  # noqa: F401
    from .entities.tile_map import TileMap  # noqa: F401
    from .core.scenes.scene import Scene  # noqa: F401
    from .core.engine import Engine  # noqa: F401
    from .core.scene_graph import SceneGraph, SceneNode, BaseGraph, BaseNode  # noqa: F401
    from .core.project import Project  # noqa: F401
    from .core.camera import Camera  # noqa: F401
    from .entities.object import Object, Transform2D, Material, create_role  # noqa: F401
    from .core.objects import register_object, object_from_dict, object_to_dict  # noqa: F401
    from .core.scenes.manager import SceneManager  # noqa: F401
    from .core.extensions import EngineExtension  # noqa: F401
    from .core.settings import EngineSettings  # noqa: F401
    from .core.effects import register_effect, get_effect, EFFECT_REGISTRY  # noqa: F401
    from .core.post_effects import (
        register_post_effect,  # noqa: F401
        get_post_effect,  # noqa: F401
        POST_EFFECT_REGISTRY,  # noqa: F401
    )
    from .core.library import (
        LibraryLoader,  # noqa: F401
        DEFAULT_LIBRARY_LOADER,  # noqa: F401
        load_engine_libraries,  # noqa: F401
    )
    from .renderers import Renderer  # noqa: F401
    from .renderers.opengl_renderer import OpenGLRenderer  # noqa: F401
    from .renderers.null_renderer import NullRenderer  # noqa: F401
    from .renderers import register_draw_handler  # noqa: F401
    from .inputs import InputManager, NullInput  # noqa: F401
    from .mesh_utils import (
        Mesh,  # noqa: F401
        create_square_mesh,  # noqa: F401
        create_triangle_mesh,  # noqa: F401
        create_circle_mesh,  # noqa: F401
    )
    from .logic.base import (
        EventSystem,  # noqa: F401
        Event,  # noqa: F401
        register_condition,  # noqa: F401
        register_action,  # noqa: F401
        get_registered_conditions,  # noqa: F401
        get_registered_actions,  # noqa: F401
        condition_from_dict,  # noqa: F401
        action_from_dict,  # noqa: F401
        event_from_dict,  # noqa: F401
    )
    from .core.resources import (
        ResourceManager,  # noqa: F401
        set_resource_root,  # noqa: F401
        get_resource_path,  # noqa: F401
    )
    from .game_window import GameWindow  # noqa: F401
    from .api import (
        load_project,  # noqa: F401
        save_project,  # noqa: F401
        run_project,  # noqa: F401
        create_engine,  # noqa: F401
        load_scene,  # noqa: F401
        save_scene,  # noqa: F401
        run_scene,  # noqa: F401
    )
    from .core.engine import main  # noqa: F401
    from .utils.units import (
        set_units_per_meter,  # noqa: F401
        meters,  # noqa: F401
        kilometers,  # noqa: F401
        to_units,  # noqa: F401
        from_units,  # noqa: F401
        set_y_up,  # noqa: F401
        Y_UP,  # noqa: F401
    )
    from .utils import units  # noqa: F401
    from .cache import Cache, SAGE_CACHE  # noqa: F401
    from .audio import AudioManager  # noqa: F401
    from .formats import (
        load_sageaudio,  # noqa: F401
        save_sageaudio,  # noqa: F401
        load_sagemesh,  # noqa: F401
        save_sagemesh,  # noqa: F401
        load_sageanimation,  # noqa: F401
        save_sageanimation,  # noqa: F401
        load_sagemap,  # noqa: F401
        save_sagemap,  # noqa: F401
        save_game,  # noqa: F401
        load_game,  # noqa: F401
    )
    from .formats import load_resource, save_resource  # noqa: F401
    from .texture_atlas import TextureAtlas  # noqa: F401
    from . import math2d  # noqa: F401
    from .version import require as require_version  # noqa: F401
