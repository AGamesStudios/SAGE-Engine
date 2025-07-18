"""Simple plugin loading system."""
from __future__ import annotations

import importlib
from types import ModuleType
from typing import Iterable

_loaded: list[str] = []


def load_plugins(modules: Iterable[str]) -> list[ModuleType]:
    loaded = []
    for name in modules:
        mod = importlib.import_module(name)
        loaded.append(mod)
        _loaded.append(name)
    return loaded


def loaded_plugins() -> list[str]:
    return list(_loaded)


__all__ = ["load_plugins", "loaded_plugins"]
