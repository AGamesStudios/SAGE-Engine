"""SAGE Engine core initialization and reset routines."""
from __future__ import annotations

import logging
import time

from types import ModuleType

from .. import dag, render, resource, ui, object as object_mod
from ..profiling import ProfileEntry, ProfileFrame

_LOGGER = logging.getLogger(__name__)

SUBSYSTEMS: dict[str, ModuleType] = {}


def register_subsystem(name: str, system: ModuleType) -> None:
    SUBSYSTEMS[name] = system


def get_subsystem(name: str) -> ModuleType:
    return SUBSYSTEMS[name]


BOOT_SEQUENCE = [
    "render",
    "resource",
    "object",
    "dag",
    "ui",
]

# Register built-in subsystems
register_subsystem("render", render)
register_subsystem("resource", resource)
register_subsystem("object", object_mod)
register_subsystem("dag", dag)
register_subsystem("ui", ui)



_booted = False
_profile: ProfileFrame | None = None


def _init_step(name: str, func) -> ProfileEntry:
    start = time.perf_counter()
    func()
    duration = (time.perf_counter() - start) * 1000.0
    print(f"[boot] SAGE {name} Init: {duration:.1f} ms")
    return ProfileEntry(name=name, ms=duration)


def core_boot() -> ProfileFrame:
    """Initialize all core subsystems and return a ProfileFrame."""
    global _booted, _profile
    if _booted:
        return _profile if _profile is not None else ProfileFrame([])

    entries: list[ProfileEntry] = []
    entries.append(_init_step("core", lambda: None))
    for name in BOOT_SEQUENCE:
        subsystem = get_subsystem(name)
        entries.append(_init_step(name, subsystem.boot))
        if name == "object":
            def _load_objects():
                mgr = resource.get_manager()
                for obj in mgr.load_all_objects("data/objects"):
                    object_mod.register_object(obj)

            entries.append(_init_step("load_objects", _load_objects))

    _profile = ProfileFrame(entries=entries)
    _booted = True
    return _profile


def core_reset() -> ProfileFrame:
    """Re-initialize the engine without restarting Python."""
    global _booted, _profile
    for name in reversed(BOOT_SEQUENCE):
        subsystem = get_subsystem(name)
        if hasattr(subsystem, "reset"):
            subsystem.reset()
    _booted = False
    _profile = None
    return core_boot()


__all__ = [
    "register_subsystem",
    "get_subsystem",
    "BOOT_SEQUENCE",
    "core_boot",
    "core_reset",
    "ProfileFrame",
    "ProfileEntry",
]
