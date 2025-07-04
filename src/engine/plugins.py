"""Plugin loading utilities for the SAGE engine and editor."""

from __future__ import annotations

import asyncio
import importlib
from importlib import metadata
import logging
import os
import sys
import json
from typing import Callable, Any

# name of the environment variable overriding the default plugin directory
PLUGIN_DIR_ENV = "SAGE_PLUGIN_DIR"

# default plugin directory in the user's home, overridable via ``SAGE_PLUGIN_DIR``
PLUGIN_DIR = os.path.expanduser(
    os.environ.get(PLUGIN_DIR_ENV, os.path.join("~", ".sage_plugins"))
)
CONFIG_FILE = os.path.join(PLUGIN_DIR, "plugins.json")


logger = logging.getLogger('sage.plugins')


def _run_sync_or_async(func: Callable, *args: Any) -> None:
    """Run *func* and await the result if it returns a coroutine."""
    res = func(*args)
    if asyncio.iscoroutine(res):
        try:
            asyncio.run(res)
        except RuntimeError:
            loop = asyncio.new_event_loop()
            asyncio.set_event_loop(loop)
            loop.run_until_complete(res)
            loop.close()


class PluginBase:
    """Base class for engine and editor plugins."""

    def init_engine(self, engine) -> None:
        """Called when the engine starts."""

    def init_editor(self, editor) -> None:
        """Called when the editor starts."""

    def register_logic(self, register_condition, register_action) -> None:
        """Optional hook to register custom logic blocks."""


class PluginManager:
    """Load and register plugins for a specific target."""

    def __init__(self, target: str,
                 *, plugin_dir: str = PLUGIN_DIR,
                 config_file: str = CONFIG_FILE,
                 entry_point_group: str | None = None) -> None:
        if target not in ("engine", "editor"):
            raise ValueError(f"Unknown plugin target: {target}")
        self.target = target
        self.plugin_dir = plugin_dir
        self.config_file = config_file
        self.entry_point_group = (
            entry_point_group or f"sage_{target}.plugins")
        self._funcs: list[Callable[[Any], Any]] = []

    def register(self, func: Callable[[Any], Any]) -> None:
        """Register a plugin initialization function."""
        self._funcs.append(func)

    def _call_init(self, mod_or_obj, instance) -> None:
        if isinstance(mod_or_obj, PluginBase):
            if self.target == "engine":
                _run_sync_or_async(mod_or_obj.init_engine, instance)
                reg = getattr(mod_or_obj, "register_logic", None)
                if callable(reg):
                    _run_sync_or_async(
                        reg,
                        getattr(instance, "register_condition", None),
                        getattr(instance, "register_action", None),
                    )
            else:
                _run_sync_or_async(mod_or_obj.init_editor, instance)
            return

        _call_init(mod_or_obj, self.target, instance)

    def load(self, instance, paths: list[str] | None = None) -> None:
        """Load plugins and initialize them with ``instance``."""
        dirs: list[str] = []
        env_all = os.environ.get("SAGE_PLUGINS")
        if env_all:
            dirs.extend(env_all.split(os.pathsep))
        env_specific = os.environ.get(
            "SAGE_EDITOR_PLUGINS" if self.target == "editor" else "SAGE_ENGINE_PLUGINS"
        )
        if env_specific:
            dirs.extend(env_specific.split(os.pathsep))
        if paths:
            dirs.extend(paths)
        dirs.append(self.plugin_dir)

        cfg = read_config(self.config_file)

        for func in list(self._funcs):
            try:
                _run_sync_or_async(func, instance)
            except Exception:
                logger.exception(
                    "Plugin function %s failed", getattr(func, "__name__", func))

        for path in dirs:
            if not path:
                continue
            if os.path.isdir(path) and path not in sys.path:
                sys.path.append(path)
            if not os.path.isdir(path):
                logger.warning("Plugin directory %s does not exist", path)
                continue
            base = os.path.realpath(path)
            for name in os.listdir(path):
                if name.startswith("_") or not name.endswith(".py"):
                    continue
                full = os.path.realpath(os.path.join(path, name))
                if not full.startswith(base + os.sep):
                    logger.warning("Ignoring plugin outside directory: %s", full)
                    continue
                mod_name = os.path.splitext(name)[0]
                if not cfg.get(mod_name, True):
                    continue
                try:
                    module = importlib.import_module(mod_name)
                    self._call_init(module, instance)
                except Exception:
                    logger.exception(
                        "Failed to load plugin %s at %s", mod_name,
                        os.path.join(path, name))

        try:
            eps = metadata.entry_points()
            entries = (
                eps.select(group=self.entry_point_group)
                if hasattr(eps, "select")
                else eps.get(self.entry_point_group, [])  # type: ignore[attr-defined]
            )
            for ep in entries:
                try:
                    mod = ep.load()
                    if callable(mod):
                        _run_sync_or_async(mod, instance)
                    else:
                        self._call_init(mod, instance)
                except Exception:
                    logger.exception("Failed to load entry point %s", ep.name)
        except Exception:
            logger.exception(
                "Error loading entry points for %s", self.entry_point_group)



# store plugin functions registered programmatically
_MANAGERS = {
    'engine': PluginManager('engine'),
    'editor': PluginManager('editor'),
}


def read_config(config_file: str = CONFIG_FILE) -> dict:
    """Return the plugin enabled state dictionary."""
    try:
        with open(config_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
            if isinstance(data, dict):
                return data
    except Exception:
        pass
    return {}


def write_config(cfg: dict, *, config_file: str = CONFIG_FILE) -> None:
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    with open(config_file, 'w', encoding='utf-8') as f:
        json.dump(cfg, f, indent=2)


def list_plugins(paths: list[str] | None = None, *, plugin_dir: str = PLUGIN_DIR,
                 config_file: str = CONFIG_FILE) -> list[tuple[str, str, bool]]:
    """Return a list of (module name, path, enabled)."""
    dirs: list[str] = []
    env_all = os.environ.get('SAGE_PLUGINS')
    if env_all:
        dirs.extend(env_all.split(os.pathsep))
    if paths:
        dirs.extend(paths)
    dirs.append(plugin_dir)

    cfg = read_config(config_file)
    result = []
    for path in dirs:
        if not os.path.isdir(path):
            continue
        base = os.path.realpath(path)
        for name in os.listdir(path):
            if name.startswith('_') or not name.endswith('.py'):
                continue
            full = os.path.realpath(os.path.join(path, name))
            if not full.startswith(base + os.sep):
                continue
            mod_name = os.path.splitext(name)[0]
            result.append((mod_name, os.path.join(path, name), cfg.get(mod_name, True)))
    return result


def register_plugin(target: str, func):
    """Register a plugin for the given target ('engine' or 'editor')."""
    if target not in _MANAGERS:
        raise ValueError(f'Unknown plugin target: {target}')
    _MANAGERS[target].register(func)


def _call_init(module, target, instance):
    """Call the init function for the requested target if present."""
    plugin_obj = getattr(module, 'plugin', None)
    if isinstance(plugin_obj, PluginBase):
        if target == 'engine':
            _run_sync_or_async(plugin_obj.init_engine, instance)
        else:
            _run_sync_or_async(plugin_obj.init_editor, instance)
        if target == 'engine':
            reg = getattr(plugin_obj, 'register_logic', None)
            if callable(reg):
                try:
                    from engine.logic import register_condition, register_action
                    _run_sync_or_async(reg, register_condition, register_action)
                except Exception:
                    logger.exception('register_logic failed in %s', module.__name__)
        return
    init_name = f'init_{target}'
    func = getattr(module, init_name, None)
    if callable(func):
        _run_sync_or_async(func, instance)
    else:
        generic = getattr(module, 'init', None)
        if callable(generic):
            _run_sync_or_async(generic, instance)
    if target == 'engine':
        reg = getattr(module, 'register_logic', None)
        if callable(reg):
            try:
                from engine.logic import register_condition, register_action
                _run_sync_or_async(reg, register_condition, register_action)
            except Exception:
                logger.exception('register_logic failed in %s', module.__name__)


def load_plugins(target: str, instance, paths=None):
    """Load plugins for the given target and initialize them with *instance*."""
    if target not in _MANAGERS:
        raise ValueError(f'Unknown plugin target: {target}')
    _MANAGERS[target].load(instance, paths)

