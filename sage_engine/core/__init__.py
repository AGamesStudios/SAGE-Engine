"""SAGE Engine core initialization and reset routines."""
from __future__ import annotations

import logging
import time
from pathlib import Path
from typing import Callable

from types import ModuleType

from .. import (
    dag,
    render,
    resource,
    ui,
    object as object_mod,
    window,
    framesync,
    input as input_mod,
    time as time_mod,
)
from sage_fs import FlowRunner
from sage import get_event_handlers
from ..lua_runner import run_lua_script, set_lua_globals
from ..python_runner import run_python_script, set_python_globals
import traceback
from ..scripts_watcher import ScriptsWatcher
from .. import perf
from sage.config import load_config
import importlib
from ..profiling import ProfileEntry, ProfileFrame
from ..logic_api import (
    create_object,
    set_param,
    get_param,
    destroy_object,
    emit_event,
    on_ready,
    on_update,
    log as logic_log,
)

_LOGGER = logging.getLogger(__name__)

SUBSYSTEM_FACTORIES: dict[str, Callable[[], ModuleType]] = {}
SUBSYSTEMS: dict[str, ModuleType] = {}


def register_subsystem(name: str, factory: Callable[[], ModuleType]) -> None:
    if name in SUBSYSTEM_FACTORIES:
        raise ValueError(f"Subsystem '{name}' already registered")
    SUBSYSTEM_FACTORIES[name] = factory


def get_subsystem(name: str) -> ModuleType:
    if name not in SUBSYSTEMS:
        factory = SUBSYSTEM_FACTORIES.get(name)
        if factory is None:
            raise KeyError(name)
        SUBSYSTEMS[name] = factory()
    return SUBSYSTEMS[name]


def load_plugins(modules: list[str]) -> None:
    for mod in modules:
        try:
            importlib.import_module(mod)
        except Exception as exc:
            print(f"[plugin] failed to load {mod}: {exc}")


BOOT_SEQUENCE = [
    "window",
    "render",
    "framesync",
    "time",
    "input",
    "resource",
    "object",
    "dag",
    "ui",
]

# Register built-in subsystems
register_subsystem("render", lambda: render)
register_subsystem("window", lambda: window)
register_subsystem("framesync", lambda: framesync)
register_subsystem("time", lambda: time_mod)
register_subsystem("input", lambda: input_mod)
register_subsystem("resource", lambda: resource)
register_subsystem("object", lambda: object_mod)
register_subsystem("dag", lambda: dag)
register_subsystem("ui", lambda: ui)



_booted = False
_profile: ProfileFrame | None = None
_watcher: ScriptsWatcher | None = None


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

    cfg = load_config()
    load_plugins(cfg.get("plugins", []))
    disabled = set(cfg.get("disabled_subsystems", []))

    entries: list[ProfileEntry] = []
    if perf.detect_low_perf():
        print("[boot] low performance mode enabled")
    entries.append(_init_step("core", lambda: None))
    for name in BOOT_SEQUENCE:
        if name in disabled:
            continue
        subsystem = get_subsystem(name)
        entries.append(_init_step(name, subsystem.boot))
        if perf.check_memory(256):
            perf.set_low_perf(True)
            print("[boot] memory high, switching to low perf")
        if name == "object":
            def _load_objects():
                mgr = resource.get_manager()
                for obj in mgr.load_all_objects("data/objects"):
                    object_mod.register_object(obj)

            entries.append(_init_step("load_objects", _load_objects))
        if name == "dag":
            def _init_flow():
                runner = FlowRunner()
                runner.context.variables.update({"input": input_mod, "time": time_mod})
                dag.register("flow.run", runner.run_file)
                dag.register("lua.run", run_lua_script)
                dag.register("python.run", run_python_script)

                set_lua_globals(
                    log=logic_log,
                    emit=emit_event,
                    create_object=create_object,
                    set_param=set_param,
                    get_param=get_param,
                    destroy_object=destroy_object,
                    on_ready=on_ready,
                    on_update=on_update,
                    input=input_mod,
                    time=time_mod,
                )
                set_python_globals(
                    log=logic_log,
                    emit=emit_event,
                    create_object=create_object,
                    set_param=set_param,
                    get_param=get_param,
                    destroy_object=destroy_object,
                    on_ready=on_ready,
                    on_update=on_update,
                    input=input_mod,
                    time=time_mod,
                )

                scripts = Path("data/scripts")
                if scripts.is_dir():
                    if cfg.get("enable_flow", True):
                        for script in scripts.rglob("*.sage_fs"):
                            runner.run_file(str(script))
                    if cfg.get("enable_lua", True):
                        for script in scripts.rglob("*.lua"):
                            run_lua_script(str(script))
                    if cfg.get("enable_python", True):
                        for script in scripts.rglob("*.py"):
                            try:
                                run_python_script(str(script))
                            except Exception as exc:
                                tb = getattr(exc, "__traceback__", None)
                                line = traceback.extract_tb(tb)[-1].lineno if tb else "?"
                                print(f"Script '{script.name}' failed at line {line}: {exc}")

                if cfg.get("watch_scripts"):
                    global _watcher
                    _watcher = ScriptsWatcher(
                        str(scripts), runner, run_lua_script, run_python_script
                    )
                    _watcher.start(1.0)

            entries.append(_init_step("load_scripts", _init_flow))

    _profile = ProfileFrame(entries=entries)
    _booted = True
    return _profile


def core_reset() -> ProfileFrame:
    """Re-initialize the engine without restarting Python."""
    global _booted, _profile, _watcher
    if _watcher:
        _watcher.stop()
        _watcher = None
    cfg = load_config()
    disabled = set(cfg.get("disabled_subsystems", []))
    for name in reversed(BOOT_SEQUENCE):
        if name in disabled:
            continue
        subsystem = get_subsystem(name)
        if hasattr(subsystem, "reset"):
            subsystem.reset()
    _booted = False
    _profile = None
    return core_boot()


def core_debug() -> None:
    """Print current objects and active event handlers for debugging."""
    print("Objects:")
    for obj in object_mod.get_objects():
        print(f" - {obj.id} role={obj.role} parent={obj.parent_id}")
    print("Events:")
    for event, handlers in get_event_handlers().items():
        print(f" - {event}: {len(handlers)} handlers")


__all__ = [
    "register_subsystem",
    "get_subsystem",
    "BOOT_SEQUENCE",
    "core_boot",
    "core_reset",
    "core_debug",
    "ProfileFrame",
    "ProfileEntry",
]

