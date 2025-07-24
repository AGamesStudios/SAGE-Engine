"""SAGE Engine Python package."""

from .core import core_boot, core_reset, core_debug, ProfileFrame
from .lua_runner import run_lua_script, set_lua_globals
from .python_runner import run_python_script, set_python_globals
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
from . import input, time
from . import perf
from . import draw, gizmo

__all__ = [
    "core_boot",
    "core_reset",
    "core_debug",
    "ProfileFrame",
    "run_lua_script",
    "set_lua_globals",
    "run_python_script",
    "set_python_globals",
    "ScriptsWatcher",
    "create_object",
    "set_param",
    "get_param",
    "destroy_object",
    "emit_event",
    "on_ready",
    "on_update",
    "input",
    "time",
    "perf",
    "draw",
    "gizmo",
]

