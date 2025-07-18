"""SAGE Engine core initialization and reset routines."""
from __future__ import annotations

from dataclasses import dataclass
import importlib
import logging
import time

from . import dag, render, resource, ui, object as object_mod

_LOGGER = logging.getLogger(__name__)

@dataclass
class InitEntry:
    name: str
    ms: float

@dataclass
class InitProfile:
    entries: list[InitEntry]


_booted = False
_profile: InitProfile | None = None


def _init_step(name: str, func) -> InitEntry:
    start = time.perf_counter()
    func()
    duration = (time.perf_counter() - start) * 1000.0
    print(f"[boot] SAGE {name} Init: {duration:.1f} ms")
    return InitEntry(name=name, ms=duration)


def core_boot() -> InitProfile:
    """Initialize all core subsystems and return an InitProfile."""
    global _booted, _profile
    if _booted:
        return _profile if _profile is not None else InitProfile([])

    entries = []
    entries.append(_init_step("Core", lambda: None))
    entries.append(_init_step("Render", render.init_render))
    entries.append(_init_step("Resources", resource.init_resource))
    entries.append(_init_step("Objects", object_mod.init_object))
    def _load_objects():
        mgr = resource.get_manager()
        objects = mgr.load_all_objects("data/objects")
        for obj in objects:
            object_mod.register_object(obj)

    entries.append(_init_step("LoadObjects", _load_objects))
    entries.append(_init_step("DAG", dag.init_dag))
    entries.append(_init_step("UI", ui.init_ui))

    _profile = InitProfile(entries=entries)
    _booted = True
    return _profile


def core_reset() -> InitProfile:
    """Re-initialize the engine without restarting Python."""
    global _booted
    _booted = False
    # try to reload modules for a clean state
    for mod in [dag, render, resource, object_mod, ui]:
        importlib.reload(mod)
    return core_boot()
