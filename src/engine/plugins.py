"""Plugin loading utilities for the SAGE engine and editor."""

from __future__ import annotations

import asyncio
import importlib
import importlib.util
from importlib import metadata
import hashlib
import logging
import os
import sys
import json
import re
from typing import Callable, Any

from engine.utils.config import get as cfg_get

# name of the environment variable overriding the default plugin directory
PLUGIN_DIR_ENV = "SAGE_PLUGIN_DIR"

# default plugin directory in the user's home
DEFAULT_PLUGIN_DIR = os.path.join("~", ".sage_plugins")


def _default_plugin_dir() -> str:
    """Return the plugin directory from configuration or ``SAGE_PLUGIN_DIR``."""
    cfg_dir = cfg_get('plugins.dir')
    if cfg_dir:
        return os.path.expanduser(str(cfg_dir))
    return os.path.expanduser(os.environ.get(PLUGIN_DIR_ENV, DEFAULT_PLUGIN_DIR))


def _default_config_file(plugin_dir: str | None = None) -> str:
    """Return the path to the plugin configuration file."""
    return os.path.join(plugin_dir or _default_plugin_dir(), "plugins.json")


logger = logging.getLogger('sage.plugins')


def _run_sync_or_async(func: Callable, *args: Any) -> None:
    """Run *func* and await the result if it returns a coroutine."""
    res = func(*args)
    if asyncio.iscoroutine(res):
        try:
            loop = asyncio.get_running_loop()
        except RuntimeError:
            asyncio.run(res)
        else:
            loop.create_task(res)


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

    def __init__(
        self,
        target: str,
        *,
        plugin_dir: str | None = None,
        config_file: str | None = None,
        entry_point_group: str | None = None,
    ) -> None:
        if target not in ("engine", "editor"):
            raise ValueError(f"Unknown plugin target: {target}")
        self.target = target
        env_dir = os.environ.get(
            "SAGE_EDITOR_PLUGIN_DIR" if target == "editor" else "SAGE_ENGINE_PLUGIN_DIR"
        )
        self.plugin_dir = os.path.expanduser(
            plugin_dir or env_dir or _default_plugin_dir()
        )
        self.config_file = os.path.expanduser(
            config_file or _default_config_file(self.plugin_dir)
        )
        self.entry_point_group = (
            entry_point_group or f"sage_{target}.plugins")
        self._funcs: list[Callable[[Any], Any]] = []
        self.loaded_modules: list[str] = []

    def register(self, func: Callable[[Any], Any]) -> None:
        """Register a plugin initialization function."""
        self._funcs.append(func)

    def _call_init(self, mod_or_obj, instance) -> None:
        """Delegate to :func:`_call_plugin_init` for consistent behaviour."""
        _call_plugin_init(mod_or_obj, self.target, instance)

    def load(self, instance, paths: list[str] | None = None) -> None:
        """Load plugins and initialize them with ``instance``."""
        importlib.invalidate_caches()
        dirs: list[str] = []
        cfg_paths = cfg_get('plugins.extra', [])
        if cfg_paths:
            dirs.extend(os.path.expanduser(p) for p in cfg_paths)
        cfg_target = cfg_get(f'plugins.{self.target}', [])
        if cfg_target:
            dirs.extend(os.path.expanduser(p) for p in cfg_target)
        env_all = os.environ.get("SAGE_PLUGINS")
        if env_all:
            dirs.extend(os.path.expanduser(p) for p in env_all.split(os.pathsep))
        env_specific = os.environ.get(
            "SAGE_EDITOR_PLUGINS" if self.target == "editor" else "SAGE_ENGINE_PLUGINS"
        )
        if env_specific:
            dirs.extend(os.path.expanduser(p) for p in env_specific.split(os.pathsep))
        else:
            dirs.append(self.plugin_dir)
        if paths:
            dirs.extend(os.path.expanduser(p) for p in paths)

        seen = set()
        dirs = [d for d in dirs if not (d in seen or seen.add(d))]

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
                    digest = hashlib.sha1(full.encode("utf-8")).hexdigest()[:8]
                    safe_name = re.sub(r"[^0-9A-Za-z_]", "_", mod_name)
                    unique = f"sage_plugins.{safe_name}_{digest}"
                    spec = importlib.util.spec_from_file_location(unique, full)
                    if spec and spec.loader:
                        module = importlib.util.module_from_spec(spec)
                        spec.loader.exec_module(module)
                        sys.modules[unique] = module
                        self.loaded_modules.append(unique)
                        self._call_init(module, instance)
                    else:
                        logger.warning("Could not load plugin spec for %s", full)
                except ModuleNotFoundError as exc:
                    logger.warning(
                        "Plugin %s requires missing dependency %s", mod_name, exc.name
                    )
                    if hasattr(instance, "log_warning"):
                        instance.log_warning(
                            f"Plugin {mod_name} requires missing dependency {exc.name}"
                        )
                except Exception as exc:
                    logger.exception(
                        "Failed to load plugin %s at %s", mod_name,
                        os.path.join(path, name))
                    if hasattr(instance, "log_warning"):
                        instance.log_warning(
                            f"Failed to load plugin {mod_name}: {exc}"
                        )

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

    def unload_all(self) -> None:
        """Unload all previously loaded modules."""
        for name in self.loaded_modules:
            sys.modules.pop(name, None)
        self.loaded_modules.clear()



# store plugin functions registered programmatically
_MANAGERS: dict[str, PluginManager] = {}


def _get_manager(target: str) -> PluginManager:
    """Return the cached :class:`PluginManager` for *target* or create it."""
    if target not in ("engine", "editor"):
        raise ValueError(f"Unknown plugin target: {target}")
    mgr = _MANAGERS.get(target)
    if mgr is None:
        mgr = PluginManager(target)
        _MANAGERS[target] = mgr
    return mgr


def read_config(config_file: str | None = None, *, plugin_dir: str | None = None) -> dict:
    """Return the plugin enabled state dictionary."""
    if config_file is None:
        config_file = _default_config_file(plugin_dir)
    try:
        with open(config_file, 'r', encoding='utf-8') as f:
            data = json.load(f)
            if isinstance(data, dict):
                return data
    except Exception as exc:
        logger.warning("Failed to read plugin config %s: %s", config_file, exc)
    return {}


def write_config(cfg: dict, *, config_file: str | None = None, plugin_dir: str | None = None) -> None:
    config_file = config_file or _default_config_file(plugin_dir)
    os.makedirs(os.path.dirname(config_file), exist_ok=True)
    with open(config_file, 'w', encoding='utf-8') as f:
        json.dump(cfg, f, indent=2)


def list_plugins(
    paths: list[str] | None = None,
    *,
    plugin_dir: str | None = None,
    config_file: str | None = None,
) -> list[tuple[str, str, bool]]:
    """Return a list of (module name, path, enabled)."""
    dirs: list[str] = []
    env_all = os.environ.get('SAGE_PLUGINS')
    if env_all:
        dirs.extend(os.path.expanduser(p) for p in env_all.split(os.pathsep))
    if paths:
        dirs.extend(os.path.expanduser(p) for p in paths)
    plugin_dir = os.path.expanduser(plugin_dir or _default_plugin_dir())
    dirs.append(plugin_dir)

    cfg = read_config(config_file, plugin_dir=plugin_dir)
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


def register_plugin(target: str, func) -> None:
    """Register a plugin for the given target ('engine' or 'editor')."""
    _get_manager(target).register(func)


def _call_plugin_init(module, target, instance):
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


def load_plugins(target: str, instance, paths=None) -> None:
    """Load plugins for the given target and initialize them with *instance*."""
    _get_manager(target).load(instance, paths)

