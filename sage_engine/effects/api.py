"""Effect registry for SAGE Effects."""

from __future__ import annotations

from typing import Callable, Dict, Any, Mapping

EffectFunc = Callable[..., None]

_registry: Dict[str, EffectFunc] = {}
_backend: str = "cpu"


def set_backend(name: str) -> None:
    """Select processing backend ('cpu' or 'gpu')."""
    global _backend
    _backend = name


def get_backend() -> str:
    return _backend


def register(name: str, func: EffectFunc) -> None:
    """Register a new effect function."""
    _registry[name] = func


def apply(name: str, buffer: bytearray, width: int, height: int, **params: Any) -> None:
    """Apply a registered effect to the buffer."""
    effect = _registry.get(name)
    if effect:
        effect(buffer, width, height, **params)


def list_effects() -> list[str]:
    return list(_registry.keys())

