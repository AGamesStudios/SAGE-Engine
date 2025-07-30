"""Simple plugin loader."""

from __future__ import annotations

import importlib.util
from pathlib import Path
from typing import Iterable, List
from ..core.extensible import IExtensible

_registered: List[IExtensible] = []


def load_plugins(directory: Path | None = None) -> Iterable[str]:
    if directory is None:
        directory = Path("plugins")
    loaded = []
    if not directory.exists():
        return loaded
    for path in directory.glob("*.py"):
        spec = importlib.util.spec_from_file_location(path.stem, path)
        if spec and spec.loader:
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            loaded.append(path.stem)
    return loaded


def register(plugin: IExtensible) -> None:
    """Register an extension and invoke its attach hook."""
    plugin.on_attach(None)
    _registered.append(plugin)


def unregister(plugin: IExtensible) -> None:
    """Unregister a previously registered extension."""
    if plugin in _registered:
        plugin.on_shutdown()
        _registered.remove(plugin)

__all__ = ["load_plugins", "register", "unregister"]
