"""Effect registry for SAGE Effects."""

from __future__ import annotations

from typing import Callable, Dict, Tuple

EffectFunc = Callable[[bytearray, int, int], None]

_registry: Dict[str, EffectFunc] = {}


def register(name: str, func: EffectFunc) -> None:
    """Register a new effect function."""
    _registry[name] = func


def apply(name: str, buffer: bytearray, width: int, height: int) -> None:
    """Apply a registered effect to the buffer."""
    effect = _registry.get(name)
    if effect:
        effect(buffer, width, height)


def list_effects() -> list[str]:
    return list(_registry.keys())

