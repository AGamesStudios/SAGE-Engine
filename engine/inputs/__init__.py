"""Input backend registry and interface for the engine."""

from __future__ import annotations

from typing import Dict, Type


class InputBackend:
    """Abstract base class for input backends."""

    def poll(self) -> None:
        raise NotImplementedError

    def is_key_down(self, key: int) -> bool:
        raise NotImplementedError

    def is_button_down(self, button: int) -> bool:
        raise NotImplementedError

    def shutdown(self) -> None:
        raise NotImplementedError


INPUT_REGISTRY: Dict[str, Type] = {}


def register_input(name: str, cls: Type) -> None:
    """Register an input backend class under ``name``."""
    INPUT_REGISTRY[name] = cls


def get_input(name: str) -> Type | None:
    """Return the input backend class associated with ``name``."""
    return INPUT_REGISTRY.get(name)


__all__ = ["InputBackend", "register_input", "get_input", "INPUT_REGISTRY"]
