"""SAGE Engine Python package."""

from .core import core_boot, core_reset, core_debug, ProfileFrame
from .lua_runner import run_lua_script, set_lua_globals
from .logic_api import (
    create_object,
    set_param,
    get_param,
    destroy_object,
    emit_event,
    on_ready,
    on_update,
)
from .scripts_watcher import ScriptsWatcher

__all__ = [
    "core_boot",
    "core_reset",
    "core_debug",
    "ProfileFrame",
    "run_lua_script",
    "set_lua_globals",
    "ScriptsWatcher",
    "create_object",
    "set_param",
    "get_param",
    "destroy_object",
    "emit_event",
    "on_ready",
    "on_update",
]

